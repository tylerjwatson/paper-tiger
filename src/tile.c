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

	for (unsigned column = 0; column < size_x; column++) {
		if ((tile_heap[column] = talloc_array(tile_heap, struct tile, size_y)) == NULL) {
			_ERROR("%s: Could not allocate Y tile heap of size %d\n", __FUNCTION__, size_y);
			ret = -1;
			goto out;
		}
	}

	*out_tiles = talloc_steal(context, tile_heap);
	ret = 0;
out:
	talloc_free(temp_context);
	return ret;
}

bool tile_active(const struct tile *tile)
{
	return (tile->s_tile_header & S_TILE_HEADER_ACTIVE) == S_TILE_HEADER_ACTIVE;
}

void tile_set_active(struct tile *tile, bool val)
{
	bit_toggle(tile->s_tile_header, 5, val);
}

uint8_t tile_colour(const struct tile *tile)
{
	return (uint8_t)(tile->s_tile_header & 31);
}

void tile_set_colour(struct tile *tile, uint8_t colour)
{
	tile->s_tile_header &= (65504 | (colour > 30 ? 30 : colour));
}

uint8_t tile_wall_colour(const struct tile *tile)
{
	return (uint8_t)(tile->b_tile_header & 31);
}

void tile_set_wall_colour(struct tile *tile, uint8_t colour)
{
	tile->s_tile_header &= (224 | (colour > 30 ? 30 : colour));
}

bool tile_honey(const struct tile *tile)
{
	return (tile->b_tile_header & B_TILE_HEADER_HONEY) == B_TILE_HEADER_HONEY;
}

void tile_set_honey(struct tile *tile, bool honey)
{
	if (honey) {
		tile->b_tile_header &= (159 | B_TILE_HEADER_HONEY);
	}
	else {
		tile->b_tile_header &= 191;
	}
}

bool tile_lava(const struct tile *tile)
{
	return (tile->b_tile_header & 32) == 32;
}

void tile_set_lava(struct tile *tile, bool lava)
{
	if (lava) {
		tile->b_tile_header &= (159 | B_TILE_HEADER_HONEY);
	}
	else {
		tile->b_tile_header &= 223;
	}
}

bool tile_half_brick(const struct tile *tile)
{
	return (tile->s_tile_header & S_TILE_HEADER_HALFBRICK) == S_TILE_HEADER_HALFBRICK;
}

void tile_set_half_brick(struct tile *tile, bool val)
{
	bit_toggle(tile->s_tile_header, 10, val);
}

void tile_set_wire(struct tile *tile, bool val)
{
	bit_toggle(tile->s_tile_header, 7, val);
}

void tile_set_wire_2(struct tile *tile, bool val)
{
	bit_toggle(tile->s_tile_header, 8, val);
}

void tile_set_wire_3(struct tile *tile, bool val)
{
	bit_toggle(tile->s_tile_header, 9, val);
}

bool tile_actuator(const struct tile *tile)
{
	return (tile->s_tile_header & S_TILE_ACTUATOR) == S_TILE_ACTUATOR;
}

void tile_set_actuator(struct tile *tile, bool val)
{
	bit_toggle(tile->s_tile_header, 11, val);
}

bool tile_inactive(const struct tile *tile)
{
	return (tile->s_tile_header & S_TILE_HEADER_INACTIVE) == S_TILE_HEADER_INACTIVE;
}

void tile_set_inactive(struct tile *tile, bool val)
{
	bit_toggle(tile->s_tile_header, 6, val);
}

uint8_t tile_slope(const struct tile *tile)
{
	return (uint8_t)((tile->s_tile_header & 28672) >> 12);
}

bool tile_wire(const struct tile *tile)
{
	return (tile->s_tile_header & S_TILE_HEADER_WIRE) == S_TILE_HEADER_WIRE;
}

bool tile_wire2(const struct tile *tile)
{
	return (tile->s_tile_header & S_TILE_HEADER_WIRE_2) == S_TILE_HEADER_WIRE_2;
}

bool tile_wire3(const struct tile *tile)
{
	return (tile->s_tile_header & S_TILE_HEADER_WIRE_3) == S_TILE_HEADER_WIRE_3;
}

void tile_copy(const struct tile *src, struct tile *dest)
{
	memcpy(dest, src, sizeof(struct tile));
}

int tile_cmp(const struct tile *src, const struct tile *dest)
{
	if (src == NULL || dest == NULL) {
		return false;
	}
	return memcmp(src, dest, sizeof(*src));
}

int tile_pack(const struct tile *tile, char *dest, uint8_t *tile_flags_1, uint8_t *tile_flags_2, uint8_t *tile_flags_3)
{
	int pos = 0;
	
	uint8_t colour, wall_colour;
	int slope = 0;

	*tile_flags_1 = *tile_flags_2 = *tile_flags_3 = 0;

	if (tile_active(tile) == true) {
		*tile_flags_1 |= 2;

		if (tile->type > 255) {
			char lsw = (char)(tile->type & 0xFF);
			pos += binary_writer_write_value(dest + pos, lsw);
			*tile_flags_1 |= 32;
		}
		else {
			char lsw = (char)(tile->type & 0xFF00) >> 8;
			pos += binary_writer_write_value(dest + pos, lsw);
		}

		if (false /*Main.tileFrameImportant[(int)tile.type]*/) {
			pos += binary_writer_write_value(dest + pos, tile->frame_x);
			pos += binary_writer_write_value(dest + pos, tile->frame_y);
		}

		if ((colour = tile_colour(tile)) != 0) {
			*tile_flags_3 |= 8;
			pos += binary_writer_write_value(dest + pos, colour);
		}
	}

	if (tile->wall != 0) {
		*tile_flags_1 |= 4;

		pos += binary_writer_write_value(dest, tile->wall);

		if ((wall_colour = tile_wall_colour(tile)) != 0) {
			*tile_flags_3 |= 16;
			pos += binary_writer_write_value(dest, wall_colour);
		}
	}

	if (tile->liquid != 0) {
		if (tile_lava(tile)) {
			*tile_flags_1 |= 16;
		} else if (tile_honey(tile)) {
			*tile_flags_1 |= 24;
		} else {
			*tile_flags_1 |= 8;
		}

		pos += binary_writer_write_value(dest, tile->liquid);
	}

	bit_toggle(*tile_flags_2, 2, tile_wire(tile));
	bit_toggle(*tile_flags_2, 3, tile_wire2(tile));
	bit_toggle(*tile_flags_2, 4, tile_wire3(tile));

	if (tile_half_brick(tile)) {
		slope = 16;
	} else if ((slope = tile_slope(tile)) != 0) {
		slope = (slope + 1) << 4;
	}

	*tile_flags_2 |= slope;

	bit_toggle(*tile_flags_3, 2, tile_actuator(tile));
	bit_toggle(*tile_flags_3, 3, tile_inactive(tile));

	if (*tile_flags_3 != 0)
	{
		*tile_flags_2 |= 1;
	}
	if (*tile_flags_2 != 0)
	{
		*tile_flags_1 |= 1;
	}

	return pos;
}