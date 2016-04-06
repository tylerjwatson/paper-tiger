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

#include "tile_section.h"

#include "../game.h"
#include "../world.h"
#include "../packet.h"
#include "../player.h"
#include "../binary_writer.h"
#include "../util.h"
#include "../rect.h"

int tile_section_new(TALLOC_CTX *ctx, const struct player *player, struct rect section, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct tile_section *tile_section;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_TILE_SECTION);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet %d\n", __FUNCTION__, PACKET_TYPE_TILE_SECTION);
		ret = -ENOMEM;
		goto out;
	}

	tile_section = talloc_zero(temp_context, struct tile_section);
	if (tile_section == NULL) {
		_ERROR("%s: out of memory tile section.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	packet->type = PACKET_TYPE_TILE_SECTION;
	
	tile_section->compressed = true;
	tile_section->x_start = section.x;
	tile_section->y_start = section.y;
	tile_section->height = section.h;
	tile_section->width = section.w;


	packet->len = PACKET_HEADER_SIZE + sizeof(struct tile_section);
	packet->data = talloc_steal(packet, tile_section);
	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int tile_section_write(const struct game *game, const struct packet *packet, uv_buf_t buffer)
{
	struct tile_section *tile_section = (struct tile_section *)packet->data;
	char *tile_buffer, *compressed_buffer;
	int ret = -1, pos = 0, tile_len = 0;
	uLongf compressed_len = 0;

	struct rect rect = rect_new(tile_section->x_start, tile_section->y_start, 
		tile_section->width, tile_section->height);
	
	tile_buffer = talloc_size(packet, 1 * 1024 * 1024);
	if (tile_buffer == NULL) {
		_ERROR("%s: out of memory allocating tile buffer @ %d,%d.\n", __FUNCTION__, rect.x, rect.y);
		return -ENOMEM;
	}

	binary_writer_write_value(buffer.base, tile_section->compressed);
	
	/*
	 * All data after the compressed is compressed.
	 */

	pos += binary_writer_write_value(tile_buffer + pos, tile_section->x_start);
	pos += binary_writer_write_value(tile_buffer + pos, tile_section->y_start);
	pos += binary_writer_write_value(tile_buffer + pos, tile_section->width);
	pos += binary_writer_write_value(tile_buffer + pos, tile_section->height);

	if (world_pack_tile_section((TALLOC_CTX *)packet, game->world, rect, tile_buffer + pos, &tile_len) < 0) {
		_ERROR("%s: cannot pack tile section @ %d,%d.\n", __FUNCTION__, rect.x, rect.y);
		return -1;
	}
	pos += tile_len;

	pos += binary_writer_write_value(tile_buffer + pos, tile_section->chest_count);

	pos += binary_writer_write_value(tile_buffer + pos, tile_section->sign_count);

	pos += binary_writer_write_value(tile_buffer + pos, tile_section->tile_entity_count);

	compressed_len = compressBound(tile_len);
	compressed_buffer = talloc_size(packet, compressed_len);
	if (compressed_buffer == NULL) {
		_ERROR("%s: out of memory allocating compression buffer.\n", __FUNCTION__);
		return -ENOMEM;
	}

	if ((ret = compress(compressed_buffer, &compressed_len, tile_buffer, pos)) != Z_OK) {
		_ERROR("%s: compression error compressing tile section: %d %s\n", __FUNCTION__, ret, zError(ret));
		return -1;
	}

	/*
	...skipping past the first two bytes solved the problem. Those bytes are part of the zlib specification (RFC 1950), 
	not the deflate specification (RFC 1951). Those bytes contain information about the compression method and flags.

	The zlib and deflate formats are related; the compressed data part of zlib-formatted data may be stored in the 
	deflate format. In particular, if the compression method in the zlib header is set to 8, then the compressed data is
	stored in the deflate format. This is true in the case of the stream you submitted, which was taken from a pdf file.

	Our DeflateStream, on the other hand, represents the deflate specification (RFC 1951) but not RFC 1950. So your 
	workaround of skipping past the first 2 bytes to get to the deflate data will definitely work.
	*/

	memcpy(buffer.base + 1, compressed_buffer + 2, compressed_len - 2);

	talloc_free(tile_buffer);

	return compressed_len + 1;
}
