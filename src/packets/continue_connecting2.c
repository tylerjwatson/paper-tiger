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

#include "packets/continue_connecting2.h"
#include "packets/world_info.h"

#include "player.h"
#include "packet.h"
#include "server.h"
#include "util.h"

int continue_connecting2_handle(struct player *player, struct packet *packet)
{
	struct packet *world_info;

	if (world_info_new(player, player, &world_info) < 0) {
		_ERROR("%s: error creating world info packet.\n", __FUNCTION__);
		return -1;
	}

	server_send_packet(player->game->server, player, world_info);

	return 0;
}
