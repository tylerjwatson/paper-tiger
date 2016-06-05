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

#include "world_section.h"

#include "tile.h"
#include "game.h"
#include "rect.h"
#include "vector_2d.h"
#include "world.h"
#include "util.h"
#include "bitmap.h"

static int __zstream_init(z_stream *stream)
{
	stream->zalloc = stream->zfree = stream->opaque = Z_NULL;

	return deflateInit(stream, Z_DEFAULT_COMPRESSION);
}

int world_section_compress(const struct world *world, unsigned section, uint8_t *buffer)
{
	struct rect tile_rect;
	struct tile *tile;
	z_stream compression_stream;
	int ret = -1, tile_pos = 0, buffer_pos = 0, have;

	uint8_t in[13 * WORLD_SECTION_WIDTH];
	uint8_t out[Z_CHUNK];

	world_section_to_tile_rect(world, section, &tile_rect);

	if (__zstream_init(&compression_stream) != Z_OK) {
		_ERROR("%s: cannot initialize zlib for compression routines.\n", __FUNCTION__);
		return -1;
	}

	for (unsigned tile_y = tile_rect.y; tile_y < tile_rect.y + WORLD_SECTION_HEIGHT; tile_y++) {
		tile_pos = 0;

		for (unsigned tile_x = tile_rect.x; tile_x < tile_rect.x + WORLD_SECTION_WIDTH; tile_x++) {
			tile = world_tile_at(world, tile_x, tile_y);
			int tile_len = tile_pack_completely(world, tile, in + tile_pos);

			if (tile_len < 0) {
				_ERROR("%s: error packing tile.\n", __FUNCTION__);
				ret = -1;
				goto out;
			}

			tile_pos += tile_len;
		}

		compression_stream.avail_in = tile_pos;
		compression_stream.next_in = in;

		do {
			compression_stream.avail_out = Z_CHUNK;
			compression_stream.next_out = out;

			ret = deflate(&compression_stream, Z_NO_FLUSH);
			have = Z_CHUNK - compression_stream.avail_out;

			memcpy(buffer + buffer_pos, out, have);
			buffer_pos += have;
		} while (compression_stream.avail_out == 0);
	}

	ret = deflate(&compression_stream, Z_FINISH);
	have = Z_CHUNK - compression_stream.avail_out;
	memcpy(buffer + buffer_pos, out, have);
	buffer_pos += have;

	ret = buffer_pos;
out:
	deflateEnd(&compression_stream);
	
	return ret;
}

static void __compress_section(uv_handle_t *handle)
{
	struct world *world = (struct world *)handle->data;

	uint8_t buffer[Z_CHUNK];

	for (unsigned section = 0; section < world->max_sections; section++) {
		

		if (bitmap_get(world->section_dirty, section)) {
			world_section_compress(world, section, buffer);
			//todo: compress section into world->section_data

			bitmap_clear(world->section_dirty, section);
			
			/*
			 * The compressor worker only compresses one section at a time per
			 * iteration of the compressor loop.
			 */
			return;
		}
	}
}

int world_section_compressor_start(struct world *world)
{
	uv_idle_start(&world->section_compress_worker, __compress_section);

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

	world->section_dirty_size = sizeof(word_t) / world->max_sections;
	dirty_table = talloc_zero_size(temp_context, world->section_dirty_size);
	if (dirty_table == NULL) {
		_ERROR("%s: out of memory allocating section dirty bitmap\n", __FUNCTION__);
		goto out;
	}

	world->section_dirty = talloc_steal(world, dirty_table);
	
	if (uv_idle_init(world->game->event_loop, &world->section_compress_worker) < 0) {
		_ERROR("%s: initializing section compress worker failed.\n", __FUNCTION__);
		goto out;
	}

	world->section_compress_worker.data = world;

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int world_section_to_coords(const struct world *world, unsigned section, struct vector_2d *out_coords)
{
	struct vector_2d coords;

	coords.x = section / world->max_sections_x;
	coords.y = section % world->max_sections_x;

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

	*out_rect = r;

	return 0;
}