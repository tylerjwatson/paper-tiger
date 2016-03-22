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

#ifndef _HAVE_WORLD_INFO_H
#define _HAVE_WORLD_INFO_H

#define PACKET_TYPE_WORLD_INFO 7

/*
 * Base length + name length + 1
 */
#define PACKET_LEN_WORLD_INFO 29

#include <uv.h>

#include "../talloc/talloc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct player;
struct packet;
	
struct world_info {
	int32_t time;
	uint8_t day_info;
	uint8_t moon_phase;
	int16_t max_tile_x;
	int16_t max_tile_y;
	int16_t spawn_tile_x;
	int16_t spawn_tile_y;
	int16_t world_surface;
	int16_t rock_layer;
	int32_t world_id;
	char *world_name;
	uint8_t moon_type;
	uint8_t bg_tree;
	uint8_t bg_corrupt;
	uint8_t bg_jungle;
	uint8_t bg_snow;
	uint8_t bg_hallow;
	uint8_t bg_crimson;
	uint8_t bg_desert;
	uint8_t bg_ocean;
	uint8_t style_ice_back;
	uint8_t style_jungle_back;
	uint8_t style_hell_back;
	float wind_speed_set;
	uint8_t num_clouds;
	int32_t tree_x[3];
	int32_t tree_style[4];
	int32_t cave_back_x[3];
	uint8_t cave_back_style[4];
	float max_raining;
	uint8_t flags_1;
	uint8_t flags_2;
	uint8_t flags_3;
	uint8_t flags_4;
	int8_t invasion_type;
	uint64_t lobby_id;
};

int world_info_new(TALLOC_CTX *ctx, const struct player *player, struct packet **out_packet);

int world_info_write(TALLOC_CTX *context, struct packet *packet, uv_buf_t *buf);

#ifdef __cplusplus
}
#endif

#endif //_HAVE_WORLD_INFO_H