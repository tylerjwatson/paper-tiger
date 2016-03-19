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

#ifndef _HAVE_CLIENT_UUID_H
#define _HAVE_CLIENT_UUID_H

#define PACKET_TYPE_CLIENT_UUID 68

/*
 * UUID length + 1
 */
#define PACKET_LEN_CLIENT_UUID 37

#include <uv.h>
#include "../packet.h"
#include "../colour.h"

#ifdef __cplusplus
extern "C" {
#endif

struct client_uuid {
	char *uuid;
};

int client_uuid_new(TALLOC_CTX *ctx, const struct player *player, const char *uuid, struct packet **out_packet);

int client_uuid_read(struct packet *packet, const uv_buf_t *buf);

int client_uuid_handle(struct player *player, struct packet *packet);
	

#ifdef __cplusplus
}
#endif

#endif //_HAVE_player_info_H