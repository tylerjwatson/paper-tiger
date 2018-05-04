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
#include "status.h"
#include "connection_complete.h"
#include "chat_message.h"

#include "../colour.h"
#include "../server.h"
#include "../world.h"
#include "../world_section.h"
#include "../game.h"
#include "../hook.h"
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
	struct packet *connection_complete;
	int section_num;

	(void)get_section;

	/*
	 * Cheat, and statically send the spawn point for now
	 */

	section_num = world_section_num_for_tile_coords(&player->game->world, player->game->world.spawn_tile.x, player->game->world.spawn_tile.y);

	game_send_world(player->game, player);

	if (connection_complete_new(player, player, &connection_complete) < 0) {
		_ERROR("%s: allocating connection complete packet failed.\n", __FUNCTION__);
		return -1;
	}

	server_send_packet(&player->game->server, player, connection_complete);

	hook_on_player_join(player->game->hooks, player->game, player);

	return 0;
}

int get_section_read(struct packet *packet)
{
	TALLOC_CTX *temp_context;
	int ret = -1, pos = 0;
	struct get_section *get_section;
	uint8_t *u_buffer = (uint8_t *)packet->data_buffer;

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
