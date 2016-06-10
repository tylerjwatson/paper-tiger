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

#define PACKET_TYPE_TILE_SECTION 10

/*
 * struct size + size of compressed tile buffer
 */
#define PACKET_LEN_TILE_SECTION 19

#include <uv.h>

#include "../talloc/talloc.h"

#include "../rect.h"

#ifdef __cplusplus
extern "C" {
#endif

struct player;
struct packet;
struct game;
	
struct tile_section {
	uint8_t compressed;
	int32_t x_start;
	int32_t y_start;
	int16_t width;
	int16_t height;

	int16_t chest_count;
	int16_t sign_count;
	int16_t tile_entity_count;
};

int tile_section_new(TALLOC_CTX *ctx, const struct player *player, unsigned section,
					 struct packet **out_packet);

int tile_section_write(const struct game *game, const struct packet *packet, uv_buf_t buffer);

int tile_section_write_v2(const struct game *game, const struct packet *packet, uv_buf_t buffer);
	
#ifdef __cplusplus
}
#endif
