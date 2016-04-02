/*
* upgraded-guacamole - A Terraria server written in C for POSIX operating systems
* Copyright (C) 2016  Tyler Watson <tyler@tw.id.au>
*
* This file is part of upgraded-guacamole.
*
* upgraded-guacamole is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.

* upgraded-guacamole is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with upgraded-guacamole.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdbool.h> 

#include "get_section.h"
#include "tile_section.h"
#include "section_tile_frame.h"
#include "connection_complete.h"
#include "chat_message.h"

#include "../colour.h"
#include "../server.h"
#include "../world.h"
#include "../game.h"
#include "../packet.h"
#include "../player.h"
#include "../colour.h"
#include "../talloc/talloc.h"
#include "../binary_reader.h"
#include "../binary_writer.h"
#include "../util.h"

#include "../config.h"

#define ARRAY_SIZEOF(a) sizeof(a)/sizeof(a[0])

int get_section_handle(struct player *player, struct packet *packet)
{
	struct get_section *get_section = (struct get_section *)packet->data;
	struct packet *section, *tile_frame, *connection_complete, *welcome_msg;
	struct rect rect, section_rect;
	
	/*
	 * Cheat, and statically send the spawn point for now
	 */

	rect.x = (player->game->world->spawn_tile.x / WORLD_SECTION_WIDTH) * WORLD_SECTION_WIDTH;
	rect.y = (player->game->world->spawn_tile.y / WORLD_SECTION_HEIGHT) * WORLD_SECTION_HEIGHT;
	rect.w = WORLD_SECTION_WIDTH;
	rect.h = WORLD_SECTION_HEIGHT;

	section_rect.x = (rect.x / WORLD_SECTION_WIDTH) - 1;
	section_rect.y = (rect.y / WORLD_SECTION_HEIGHT) - 1;
	section_rect.h = section_rect.y + 2;
	section_rect.w = section_rect.x + 2;

	if (tile_section_new(player, player, rect, &section) < 0) {
		_ERROR("%s: allocating tile section failed.\n", __FUNCTION__);
		return -1;
	}

	if (section_tile_frame_new(player, player, section_rect, &tile_frame) < 0) {
		_ERROR("%s: allocating tile frame section failed.\n", __FUNCTION__);
		return -1;
	}

	if (connection_complete_new(player, player, &connection_complete) < 0) {
		_ERROR("%s: allocating connection complete packet failed.\n", __FUNCTION__);
		return -1;
	}
	
	if (chat_message_new(player, player, colour_black, "testicles", &welcome_msg) < 0) {
		_ERROR("%s: allocating connection complete packet failed.\n", __FUNCTION__);
		return -1;
	}

	((struct chat_message *)welcome_msg->data)->id = 0xFF;
	
	server_send_packet(player->game->server, player, section);
	//_sleep(100);
	server_send_packet(player->game->server, player, tile_frame);
	//_sleep(100);
	server_send_packet(player->game->server, player, connection_complete);
	
	game_send_message(player->game, player, colour_black, "Welcome to %s v%d.%d.", 
		PRODUCT_NAME, VERSION_MAJOR, VERSION_MINOR);
	
	return 0;
}

int get_section_read(struct packet *packet, const uv_buf_t *buf)
{
	TALLOC_CTX *temp_context;
	int ret = -1, pos = 0;
	struct get_section *get_section;
	uint8_t *u_buffer = (uint8_t *)buf->base;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for get section.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	get_section = talloc_zero(temp_context, struct get_section);
	if (get_section == NULL) {
		_ERROR("%s: out of memory allocating get section.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	get_section->x = *(int32_t *)(u_buffer + pos);
	pos += sizeof(int32_t);
	get_section->y = *(int32_t *)(u_buffer + pos);

	packet->data = (void *)talloc_steal(packet, get_section);

	ret = 0;
out:
	talloc_free(temp_context);
	
	return ret;
}