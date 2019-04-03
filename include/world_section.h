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

#include "talloc/talloc.h"
#include <stdint.h>

#define Z_CHUNK 65535
#define WORLD_SECTION_TO_OFFSET(world, x, y) y * world->max_sections_y + x

struct rect;
struct vector_2d;
struct world;

struct world_section_data {
	unsigned section;
	unsigned len;
	uint8_t data[Z_CHUNK];
};

int
world_section_init(TALLOC_CTX *context, struct world *world);

int
world_section_compressor_start(struct world *world);

struct vector_2d
world_section_num_to_coords(const struct world *world, unsigned section);

int
world_section_to_tile_rect(const struct world *world, unsigned section, struct rect *out_rect);

struct rect
world_section_for_tile_coords(const struct world *world, uint16_t tile_x, uint16_t tile_y);

unsigned int
world_section_num_for_tile_coords(const struct world *world, uint16_t tile_x, uint16_t tile_y);
