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

#define PACKET_TYPE_SECTION_TILE_FRAME 11

/*
 * struct size + size of compressed tile buffer
 */
#define PACKET_LEN_SECTION_TILE_FRAME 8

#include <uv.h>
#include <stdint.h>
#include "talloc/talloc.h"

#include "game.h"
#include "vector_2d.h"

#ifdef __cplusplus
extern "C" {
#endif

struct player;
struct packet;

struct section_tile_frame {
	int16_t x;
	int16_t y;
	int16_t dx;
	int16_t dy;
};

int section_tile_frame_new(TALLOC_CTX *ctx, const struct player *player, struct vector_2d coords,
							struct packet **out_packet);


int section_tile_frame_write(const ptGame *game, struct packet *packet);

#ifdef __cplusplus
}
#endif
