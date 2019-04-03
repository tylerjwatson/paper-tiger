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

#pragma once

#include <stdint.h>
#include <uv.h>

#include "game.h"
#include "colour.h"
#include "item.h"
#include "talloc/talloc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct packet;

struct player_stats {
	uint8_t skin_variant;
	uint8_t hair;

	uint8_t hair_dye;
	uint8_t hide_visuals;
	uint8_t hide_visuals2;
	uint8_t hide_misc;

	struct colour hair_colour;
	struct colour skin_colour;
	struct colour eye_colour;
	struct colour shirt_colour;
	struct colour under_shirt_colour;
	struct colour pants_colour;
	struct colour shoe_colour;

	uint8_t difficulty;
};

struct player {
	uint32_t id;
	char *name;
	char *uuid;
	char *remote_addr;
	uint16_t remote_port;
	ptGame *game;
	uv_tcp_t *handle;

	uint16_t life;
	uint16_t life_max;
	uint16_t mana;
	uint16_t mana_max;

	struct item_slot inventory[180];

	struct player_stats stats;

	struct packet *incoming_packet;
};

int
player_new(TALLOC_CTX *context, const ptGame *game, int id, struct player **out_player);

void
player_close(struct player *player);

#ifdef __cplusplus
}
#endif
