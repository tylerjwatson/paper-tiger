/*
 * paper-tiger - A Terraria server written in C for POSIX operating systems
 * Copyright (C) 2016  Tyler Watson <tyler@tw.id.au>
 *
 * This file is part of paper-tiger.
 * 
 * paper-tiger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * paper-tiger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with paper-tiger.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <zlib.h>
#include <assert.h>

#include "world_section.h"

#include "world.h"
#include "tile.h"
#include "game.h"
#include "rect.h"
#include "vector_2d.h"
#include "util.h"
#include "bitmap.h"
#include "binary_writer.h"

static int __zstream_init(z_stream *stream)
{
	stream->zalloc = Z_NULL;
	stream->zfree = Z_NULL;
	stream->opaque = Z_NULL;

	return deflateInit(stream, Z_DEFAULT_COMPRESSION);
}

int world_section_compress(const struct world *world, unsigned section, uint8_t *buffer)
{
	struct rect tile_rect;
	struct tile *tile;
	z_stream compression_stream;
	int ret = -1, in_pos = 0, buffer_pos = 0, have;

	uint8_t in[13 * WORLD_SECTION_WIDTH];
	uint8_t out[Z_CHUNK];

	world_section_to_tile_rect(world, section, &tile_rect);

	/*
	 * The section header rectangle must be written to the input first
	 * before the tile stream.
	 */
	
	in_pos += binary_writer_write_value(in + in_pos, tile_rect.x);
	in_pos += binary_writer_write_value(in + in_pos, tile_rect.y);
	in_pos += binary_writer_write_value(in + in_pos, tile_rect.w);
	in_pos += binary_writer_write_value(in + in_pos, tile_rect.h);
	
	if (__zstream_init(&compression_stream) != Z_OK) {
		_ERROR("%s: cannot initialize zlib for compression routines.\n", __FUNCTION__);
		return -1;
	}

	for (unsigned tile_y = tile_rect.y; tile_y < tile_rect.y + WORLD_SECTION_HEIGHT; tile_y++) {
		for (unsigned tile_x = tile_rect.x; tile_x < tile_rect.x + WORLD_SECTION_WIDTH; tile_x++) {
			tile = world_tile_at((struct world *)world, tile_x, tile_y);
			int tile_len = tile_pack_completely(world, tile, &in[in_pos]);

			if (tile_len < 0) {
				_ERROR("%s: error packing tile.\n", __FUNCTION__);
				ret = -1;
				goto out;
			}

			in_pos += tile_len;
		}

		compression_stream.avail_in = in_pos;
		compression_stream.next_in = in;

		do {
			compression_stream.avail_out = Z_CHUNK;
			compression_stream.next_out = out;

			ret = deflate(&compression_stream, Z_NO_FLUSH);
			have = Z_CHUNK - compression_stream.avail_out;

			memcpy(&buffer[buffer_pos], out, have);
			buffer_pos += have;
		} while (compression_stream.avail_out == 0);
		in_pos = 0;
	}
	
	/*
	 * Tile entity count, chest count and sign count
	 */
	compression_stream.avail_in = 6;
	memset(in, 0, 6);
	compression_stream.next_in = in;
	
	do {
		compression_stream.avail_out = Z_CHUNK;
		compression_stream.next_out = out;
		
		ret = deflate(&compression_stream, Z_FINISH);
		have = Z_CHUNK - compression_stream.avail_out;
		
		memcpy(&buffer[buffer_pos], out, have);
		buffer_pos += have;
	} while (compression_stream.avail_out == 0);
	
	ret = buffer_pos;
out:
	deflateEnd(&compression_stream);
	
	return ret;
}

static void __compress_section(uv_timer_t *handle)
{
	struct world *world = (struct world *)handle->data;
	struct world_section_data *section_data;
	
	uint8_t buffer[Z_CHUNK];
	int section_len;
	
	for (unsigned section = 0; section < world->max_sections; section++) {
		if (bitmap_get(world->section_dirty, section) == false) {
			continue;
		}
		/*
		 * Note:
		 *
		 * A staging buffer is used here for the call to world_section_compress
		 * instead of world->section_data directly because the compression may
		 * fail.  In such a case, we don't want good section tile data exchanged
		 * with a half-munted turd from a failed compression round.
		 *
		 * If a section fails to zcompress then it remains dirty and will be tackled
		 * again next round.
		 */
		 
		//printf("compressing section %d\n", section);
		section_len = world_section_compress(world, section, buffer);
		if (section_len < 0) {
			_ERROR("%s: zcompressor error compressing section %d.\n", __FUNCTION__, section);
			continue;
		}
		
		section_data = &world->section_data[section];
		memcpy(section_data->data, buffer, section_len);
		section_data->len = section_len;
		
		bitmap_clear(world->section_dirty, section);
		
		/*
		 * The compressor worker only compresses one section at a time per
		 * iteration of the compressor loop.
		 */
		return;
	}
}

int world_section_compressor_start(struct world *world)
{
	uv_timer_start(&world->section_compress_worker, __compress_section, 0, 100);
	//uv_idle_start(&world->section_compress_worker, __compress_section);

	return 0;
}

static int world_section_init_section_data(struct world *world)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct world_section_data *section_data;
	
	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for section data.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	section_data = talloc_zero_array(temp_context, struct world_section_data, world->max_sections);
	if (section_data == NULL) {
		_ERROR("%s: out of memory allocating section data array.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	for(unsigned i = 0; i < world->max_sections; i++) {
		section_data[i].section = i;
	}
	
	world->section_data = talloc_steal(world, section_data);
	
	ret = 0;
out:
	talloc_free(temp_context);
	return ret;
}

static int world_section_compress_all(struct world *world)
{
	struct world_section_data *section_data;
	
	uint8_t buffer[Z_CHUNK];
	int section_len;
	
	for (unsigned section = 0; section < world->max_sections; section++) {
		/*
		 * Note:
		 *
		 * A staging buffer is used here for the call to world_section_compress
		 * instead of world->section_data directly because the compression may
		 * fail.  In such a case, we don't want good section tile data exchanged
		 * with a half-munted turd from a failed compression round.
		 *
		 * If a section fails to zcompress then it remains dirty and will be tackled
		 * again next round.
		 */
		
		section_len = world_section_compress(world, section, buffer);
		if (section_len < 0) {
			_ERROR("%s: zcompressor error compressing section %d.\n", __FUNCTION__, section);
			return -1;
		}
		
		section_data = &world->section_data[section];
		
		memcpy(section_data->data, buffer, section_len);
		section_data->len = section_len;
	}
	
	return 0;
}

int world_section_init(struct world *world)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	word_t *dirty_table;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		ret = -ENOMEM;
		_ERROR("%s: out of memory allocating temp context.\n", __FUNCTION__);
		goto out;
	}

	world->section_dirty_size = world->max_sections / sizeof(word_t);
	dirty_table = talloc_zero_size(temp_context, world->section_dirty_size);
	if (dirty_table == NULL) {
		_ERROR("%s: out of memory allocating section dirty bitmap\n", __FUNCTION__);
		goto out;
	}

	world->section_dirty = talloc_steal(world, dirty_table);
	
	if (uv_timer_init(world->game->event_loop, &world->section_compress_worker) < 0) {
		_ERROR("%s: initializing section compress worker failed.\n", __FUNCTION__);
		goto out;
	}

	world->section_compress_worker.data = world;

	if (world_section_init_section_data(world) < 0) {
		_ERROR("%s: init section data failed.\n", __FUNCTION__);
		goto out;
	}
	
	if (world_section_compress_all(world) < 0) {
		_ERROR("%s: compressing section data failed.\n", __FUNCTION__);
		goto out;
	}
	
	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int world_section_to_coords(const struct world *world, unsigned section, struct vector_2d *out_coords)
{
	struct vector_2d coords;

	coords.x = section / world->max_sections_y;
	coords.y = section % world->max_sections_y;

	*out_coords = coords;

	return 0;
}

int world_section_to_tile_rect(const struct world *world, unsigned section, struct rect *out_rect)
{
	struct rect r;
	struct vector_2d section_coords;

	if (section > world->max_sections) {
		return -1;
	}

	world_section_to_coords(world, section, &section_coords);

	r.x = section_coords.x * WORLD_SECTION_WIDTH;
	r.y = section_coords.y * WORLD_SECTION_HEIGHT;
	r.w = WORLD_SECTION_WIDTH;
	r.h	= WORLD_SECTION_HEIGHT;

	*out_rect = r;

	return 0;
}

struct rect world_section_for_tile_coords(const struct world *world, uint16_t tile_x, uint16_t tile_y)
{
	struct rect top_rect;
	
	top_rect.x = (tile_x / WORLD_SECTION_WIDTH) * WORLD_SECTION_WIDTH;
	top_rect.y = (tile_y / WORLD_SECTION_HEIGHT) * WORLD_SECTION_HEIGHT;
	top_rect.w = WORLD_SECTION_WIDTH;
	top_rect.h = WORLD_SECTION_HEIGHT;
	
	return top_rect;
}

static unsigned int world_section_coords_to_num(const struct world *world, unsigned x, unsigned y)
{
	return world->max_sections_y * x + y;
}

unsigned int world_section_num_for_tile_coords(const struct world *world, uint16_t tile_x, uint16_t tile_y)
{
	struct rect tile_coords;
	unsigned sec;
	
	tile_coords = world_section_for_tile_coords(world, tile_x, tile_y);
	tile_coords.x /= WORLD_SECTION_WIDTH;
	tile_coords.y /= WORLD_SECTION_HEIGHT;
	
	sec = world_section_coords_to_num(world, tile_coords.x, tile_coords.y);
	
	return sec;
}