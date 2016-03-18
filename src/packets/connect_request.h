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

#ifndef _HAVE_CONNECT_REQUEST_H
#define _HAVE_CONNECT_REQUEST_H

#define PACKET_TYPE_CONNECT_REQUEST 0x01

#include <uv.h>
#include "../packet.h"
#include "../player.h"

#ifdef __cplusplus
extern "C" {
#endif

struct connect_request {
	struct packet *packet;
	char *protocol_version;
};

int connect_request_read(struct packet *packet, const uv_buf_t *buf);

int connect_request_handle(const struct player *player, struct packet *packet);

#ifdef __cplusplus
}
#endif

#endif //_HAVE_CONNECT_REQUEST_H