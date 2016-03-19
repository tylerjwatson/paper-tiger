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
 * 
 * upgraded-guacamole is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with upgraded-guacamole.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _HAVE_PLAYER_H
#define _HAVE_PLAYER_H

#include <stdint.h>
#include <uv.h>

#include "talloc/talloc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct packet;
struct game;
	
struct player {
    uint32_t id;
	char *name;
	char *uuid;
	char *remote_addr;
	uint16_t remote_port;
	struct game *game;
    uv_tcp_t *handle;
	
	uint16_t life;
	uint16_t life_max;
	uint16_t mana;
	uint16_t mana_max;
	
	struct packet *incoming_packet;
};

int player_new(TALLOC_CTX *context, const struct game *game, int id, struct player **out_player);

void player_close(struct player *player);

#ifdef __cplusplus
}
#endif

#endif /* _HAVE_PLAYER_H */