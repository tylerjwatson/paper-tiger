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

#pragma once

#define PACKET_TYPE_CHAT_MESSAGE 25

/*
 * 3 + message length
 */
#define PACKET_LEN_CHAT_MESSAGE 3

#include <uv.h>

#include "../colour.h"
#include "../talloc/talloc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct packet;
struct player;
struct game;
  
struct chat_message {
	uint8_t id;
	struct colour colour;
	char *message;
};

int chat_message_new(TALLOC_CTX *ctx, const struct player *player, const struct colour colour,
					   const char *message, struct packet **out_packet);

int chat_message_read(struct packet *packet);

int chat_message_handle(struct player *player, struct packet *packet);

int chat_message_write(const struct game *game, struct packet *packet);

#ifdef __cplusplus
}
#endif
