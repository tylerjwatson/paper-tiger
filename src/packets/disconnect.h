/*
* paper-tiger - A Terraria server written in C for POSIX operating systems
* Copyright (C) 2016  Tyler Watson <tyler@tw.id.au>
* Copyright (C) 2016  DeathCradle <rt.luke.s@gmail.com>
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

#pragma once

#define PACKET_TYPE_DISCONNECT 0x02

#include <uv.h>

#include "../talloc/talloc.h"

struct packet;
struct player;
struct game;

#ifdef __cplusplus
extern "C" {
#endif

struct player;
struct packet;

struct disconnect {
	char *reason;
};

int disconnect_new(TALLOC_CTX *ctx, const struct player *player, const char *reason, struct packet **out_packet);

int disconnect_write(const struct game *game, const struct packet *packet, uv_buf_t buf);
	
int disconnect_read(struct packet *packet, const uv_buf_t *buf);

int disconnect_handle(struct player *player, struct packet *packet);


#ifdef __cplusplus
}
#endif