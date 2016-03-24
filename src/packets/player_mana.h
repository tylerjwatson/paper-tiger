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

#ifndef _HAVE_PLAYER_MANA_H
#define _HAVE_PLAYER_MANA_H

#define PACKET_TYPE_PLAYER_MANA 42
#define PACKET_LEN_PLAYER_MANA 5

#include <uv.h>

#include "../talloc/talloc.h"
#include "../colour.h"

#ifdef __cplusplus
extern "C" {
#endif

struct player;
struct packet;
	
struct player_mana {
	uint8_t id;
	uint16_t mana;
	uint16_t mana_max;
};

int player_mana_new(TALLOC_CTX *ctx, const struct player *player, uint16_t mana, uint16_t mana_max, struct packet **out_packet);

int player_mana_write(TALLOC_CTX *context, struct packet *packet, uv_buf_t *buf);

int player_mana_read(struct packet *packet, const uv_buf_t *buf);

int player_mana_handle(struct player *player, struct packet *packet);
	

#ifdef __cplusplus
}
#endif

#endif //_HAVE_PLAYER_MANA_H