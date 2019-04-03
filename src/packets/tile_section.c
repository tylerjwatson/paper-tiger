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

#include "packets/tile_section.h"

#include "world.h"
#include "world_section.h"
#include "packet.h"
#include "player.h"
#include "binary_writer.h"
#include "util.h"
#include "rect.h"

int tile_section_new(TALLOC_CTX *ctx, const struct player *player, unsigned section, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct tile_section *tile_section;
	struct vector_2d section_coords;

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

	section_coords = world_section_num_to_coords(player->game->world, section);

	packet->type = PACKET_TYPE_TILE_SECTION;

	tile_section->compressed = true;
	tile_section->x_start = section_coords.x * WORLD_SECTION_WIDTH;
	tile_section->y_start = section_coords.y * WORLD_SECTION_HEIGHT;
	tile_section->height = WORLD_SECTION_HEIGHT;
	tile_section->width = WORLD_SECTION_WIDTH;


	packet->len = PACKET_HEADER_SIZE + sizeof(struct tile_section);
	packet->data = talloc_steal(packet, tile_section);
	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int tile_section_write_v2(const ptGame *game, struct packet *packet)
{
	struct tile_section *tile_section = (struct tile_section *)packet->data;
	int pos = 0;
	unsigned section_num;

	section_num = world_section_num_for_tile_coords(game->world, tile_section->x_start,
													tile_section->y_start);

	pos += binary_writer_write_value(packet->data_buffer, tile_section->compressed);

	/*
	 * Skipping 2 bytes of GZip header here
	 */
	memcpy(&packet->data_buffer[pos], game->world->section_data[section_num].data[2],
		   game->world->section_data[section_num].len - 2);

	pos += game->world->section_data[section_num].len;
	pos -= 2;

	return pos;
}

