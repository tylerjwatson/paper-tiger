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

#include <string.h>

#include "tile.h"
#include "world.h"
#include "util.h"
#include "binary_writer.h"

int tile_heap_new(TALLOC_CTX *context, const uint32_t size_x, const uint32_t size_y, struct tile ***out_tiles)
{
	TALLOC_CTX *temp_context;
	int ret;
	struct tile **tile_heap;

	if ((temp_context = talloc_new(NULL)) == NULL) {
		_ERROR("%s: Could not allocate talloc context for tile heap.", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if ((tile_heap = talloc_array(temp_context, struct tile *, size_x)) == NULL) {
		_ERROR("%s: Could not allocate X tile heap of size %d\n", __FUNCTION__, size_x);
		ret = -1;
		goto out;
	}

	 for(unsigned column = 0; column < size_x; column++) {
		 if ((tile_heap[column] = talloc_array(tile_heap, struct tile, size_y)) == NULL) {
			_ERROR("%s: Could not allocate Y tile heap of size %d\n", __FUNCTION__, size_y);
			ret = -1;
			goto out;
		}
	}

//	*out_tiles = tile_heap;
	 *out_tiles = talloc_steal(context, tile_heap);
	 ret = 0;
out:
	 talloc_free(temp_context);
	 return ret;
}

bool tile_active(struct tile *tile)
{
	return (tile->s_tile_header & S_TILE_HEADER_ACTIVE) == S_TILE_HEADER_ACTIVE;
}

void tile_set_active(struct tile *tile, bool val)
{
	if (val) {
		tile->s_tile_header |= S_TILE_HEADER_ACTIVE;
	} else {
		tile->s_tile_header &= ~S_TILE_HEADER_ACTIVE;
	}
}

uint8_t tile_colour(struct tile *tile)
{
	return (uint8_t)(tile->s_tile_header & 31);
}

void tile_set_colour(struct tile *tile, uint8_t colour)
{
	tile->s_tile_header &= (65504 | (colour > 30 ? 30 : colour));
}

uint8_t tile_wall_colour(struct tile *tile)
{
	return (uint8_t)(tile->b_tile_header & 31);
}

void tile_set_wall_colour(struct tile *tile, uint8_t colour)
{
	tile->s_tile_header &= (224 | (colour > 30 ? 30 : colour));
}

bool tile_honey(struct tile *tile)
{
	return (tile->b_tile_header & B_TILE_HEADER_HONEY) == B_TILE_HEADER_HONEY;
}

void tile_set_honey(struct tile *tile, bool honey)
{
	if (honey) {
		tile->b_tile_header &= (159 | B_TILE_HEADER_HONEY);
	} else {
		tile->b_tile_header &= 191;
	}
}

bool tile_lava(struct tile *tile)
{
	return (tile->b_tile_header & 32) == 32;
}

void tile_set_lava(struct tile *tile, bool lava)
{
	if (lava) {
		tile->b_tile_header &= (159 | B_TILE_HEADER_HONEY);
	} else {
		tile->b_tile_header &= 223;
	}
}

void tile_set_wire(struct tile *tile, bool tile_val)
{
	if (tile_val) {
		tile->s_tile_header |= S_TILE_HEADER_WIRE;
	} else {
		tile->s_tile_header &= ~S_TILE_HEADER_WIRE;
	}
}

void tile_set_wire_2(struct tile *tile, bool tile2)
{
	if (tile2) {
		tile->s_tile_header |= S_TILE_HEADER_WIRE_2;
	} else {
		tile->s_tile_header &= ~S_TILE_HEADER_WIRE_2;
	}
}

void tile_set_wire_3(struct tile *tile, bool tile3)
{
	if (tile3) {
		tile->s_tile_header |= S_TILE_HEADER_WIRE_3;
	} else {
		tile->s_tile_header &= ~S_TILE_HEADER_WIRE_3;
	}
}

void tile_set_actuator(struct tile *tile, bool val)
{
	if (val) {
		tile->s_tile_header |= S_TILE_ACTUATOR;
	} else {
		tile->s_tile_header &= ~S_TILE_ACTUATOR;
	}
}

void tile_set_inactive(struct tile *tile, bool val)
{
	if (val) {
		tile->s_tile_header |= S_TILE_ACTUATOR;
	} else {
		tile->s_tile_header &= 65471;
	}
}

void tile_copy(const struct tile *src, struct tile *dest)
{
	memcpy(dest, src, sizeof(struct tile));
}

int tile_pack(const struct tile *tile, char *dest)
{
	char flag_1;

	if (tile->type);
}