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

#include "section_tile_frame.h"

#include "../game.h"
#include "../world.h"
#include "../packet.h"
#include "../player.h"
#include "../binary_writer.h"
#include "../util.h"
#include "../rect.h"

int section_tile_frame_new(TALLOC_CTX *ctx, const struct player *player, struct vector_2d coords, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct section_tile_frame *section_tile_frame;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_SECTION_TILE_FRAME);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet %d\n", __FUNCTION__, PACKET_TYPE_SECTION_TILE_FRAME);
		ret = -ENOMEM;
		goto out;
	}

	section_tile_frame = talloc_zero(temp_context, struct section_tile_frame);
	if (section_tile_frame == NULL) {
		_ERROR("%s: out of memory tile section.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	packet->type = PACKET_TYPE_SECTION_TILE_FRAME;
	
	section_tile_frame->x = coords.x;
	section_tile_frame->y = coords.y;
	section_tile_frame->dx = coords.x + 1;
	section_tile_frame->dy = coords.y + 1;


	packet->len = PACKET_HEADER_SIZE + PACKET_LEN_SECTION_TILE_FRAME;
	packet->data = talloc_steal(packet, section_tile_frame);
	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int section_tile_frame_write(const struct game *game, const struct packet *packet, uv_buf_t buffer)
{
	struct section_tile_frame *section_tile_frame = (struct section_tile_frame *)packet->data;
	int pos = 0;

	pos += binary_writer_write_value(buffer.base + pos, section_tile_frame->x);
	pos += binary_writer_write_value(buffer.base + pos, section_tile_frame->y);
	pos += binary_writer_write_value(buffer.base + pos, section_tile_frame->dx);
	pos += binary_writer_write_value(buffer.base + pos, section_tile_frame->dy);

	return pos;
}
