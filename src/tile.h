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
 * 
 * paper-tiger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with paper-tiger.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _HAVE_TILE_H
#define _HAVE_TILE_H

#include <stdint.h>

#include "talloc/talloc.h"
#include "binary_reader.h"

#define TILE_SECTION_WIDTH 100

#ifdef __cplusplus
extern "C" {
#endif

struct game;

enum {
	WORLD_FILE_TILE_HAS_FLAGS = 1,
	WORLD_FILE_TILE_ACTIVE = 1 << 1,
	WORLD_FILE_TILE_IS_WALL = 1 << 2,
	WORLD_FILE_TYPE_SHORT = 1 << 5
};

enum {
	B_TILE_HEADER_HONEY = 1 << 6
};

enum {
	S_TILE_HEADER_LAVA = 1 << 4,
	S_TILE_HEADER_ACTIVE = 1 << 5,
	S_TILE_HEADER_INACTIVE = 1 << 6,
	S_TILE_HEADER_WIRE = 1 << 7,
	S_TILE_HEADER_WIRE_2 = 1 << 8,
	S_TILE_HEADER_WIRE_3 = 1 << 9,
	S_TILE_HEADER_HALFBRICK = 1 << 10,
	S_TILE_ACTUATOR = 1 << 11
};

enum {
	TILE_IS_WIRE = 1,
	TILE_WIRE_1 = 1 << 1,
	TILE_WIRE_2 = 1 << 2,
	TILE_WIRE_3 = 1 << 3,
};

enum {
	WORLD_FILE_TILE_COLOUR_ACTUATOR = 1 << 1,
	WORLD_FILE_TILE_COLOUR_INACTIVE = 1 << 2,
	WORLD_FILE_TILE_COLOUR = 1 << 3,
	WORLD_FILE_WALL_COLOUR = 1 << 4
};


struct tile {
	uint16_t type;
	uint8_t wall;
	uint8_t liquid;
	int16_t s_tile_header;
	int8_t b_tile_header;
	int8_t b_tile_header_2;
	int8_t b_tile_header_3;
	int16_t frame_x;
	int16_t frame_y;
};

struct tile_row {
	int x;
	struct tile *tiles;
};


int tile_heap_new(TALLOC_CTX *context, const uint32_t size_x, const uint32_t size_y, struct tile ***out_tiles);

bool tile_active(const struct tile *tile);
void tile_set_active(struct tile *tile, bool val);

uint8_t tile_colour(const struct tile *tile);
void tile_set_colour(struct tile *tile, uint8_t colour);

uint8_t tile_wall_colour(const struct tile *tile);
void tile_set_wall_colour(struct tile *tile, uint8_t colour);

bool tile_honey(const struct tile *tile);
void tile_set_honey(struct tile *tile, bool honey);

bool tile_lava(const struct tile *tile);
void tile_set_lava(struct tile *tile, bool lava);

void tile_set_wire(struct tile *tile, bool tile_val);
void tile_set_wire_2(struct tile *tile, bool tile2);
void tile_set_wire_3(struct tile *tile, bool tile3);

void tile_set_inactive(struct tile *tile, bool val);
void tile_set_actuator(struct tile *tile, bool val);

void tile_copy(const struct tile *src, struct tile *dest);

int tile_pack(const struct game *game, const struct tile *tile, char *dest, uint8_t *tile_flags_1, 
			  uint8_t *tile_flags_2, uint8_t *tile_flags_3);

int tile_cmp(const struct tile *src, const struct tile *dest);

#ifdef __cplusplus
}
#endif

#endif
