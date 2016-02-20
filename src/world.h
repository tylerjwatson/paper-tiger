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

#ifndef _HAVE_WORLD_H
#define _HAVE_WORLD_H

#include <talloc.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

#include "binary_reader.h"

struct world_flags {
	bool crimson;
	bool downed_boss_1;
	bool downed_boss_2;
	bool downed_boss_3;
	bool downed_queen_bee;
	bool downed_mech_1;
	bool downed_mech_2;
	bool downed_mech_3;
	bool downed_mech_any;
	bool downed_plant;
	bool downed_golem;
	bool downed_slime_king;
	bool downed_goblins;
	bool downed_clowns;
	bool downed_frost;
	bool downed_pirates;
	bool downed_fishron;
	bool downed_martians;
	bool downed_ancient_cultist;
	bool downed_moonlord;
	bool downed_halloween_king;
	bool downed_halloween_tree;
	bool downed_christmas_ice_queen;
	bool downed_christmas_santank;
	bool downed_christmas_tree;
	bool downed_tower_solar;
	bool downed_tower_vortex;
	bool downed_tower_nebula;
	bool downed_tower_stardust;
	bool active_tower_solar;
	bool active_tower_vortex;
	bool active_tower_nebula;
	bool active_tower_stardust;
	bool lunar_apocalypse_up;
	bool saved_goblin;
	bool saved_wizard;
	bool saved_mech;
	bool shadow_orb_smashed;
	bool spawn_meteor;
	bool hard_mode;
	bool raining;
};

struct vector_2d {
	int32_t x;
	int32_t y;
};

enum relogic_file_type {
	relogic_file_type_none,
	relogic_file_type_map,
	relogic_file_type_world,
	relogic_file_type_player
};

struct world {
	int32_t worldID;
	char *world_name;		/* World name */
	char *world_path;		/* Fully-qualified path to the world file */
	int version;
	uint16_t num_positions;		/* Number of elements in the positions array */
	int32_t *positions;			/* Array of position data */
	uint16_t num_important;		/* Number of elements in the importance array */
	int8_t *important;		/* Array of importance data */

	float left_world;
	float right_world;
	float top_world;
	float bottom_world;

	int32_t max_tiles_x;
	int32_t max_tiles_y;

	int64_t creation_time;

	bool expert_mode;
	int8_t moon_type;
	int32_t tree_x[3];
	int32_t tree_style[4];
	int32_t cave_back_x[3];
	int32_t cave_back_style[4];
	int32_t ice_back_style;
	int32_t jungle_back_style;
	int32_t hell_back_style;
	struct vector_2d spawn_tile;
	double world_surface;
	double rock_layer;
	double temp_time;
	bool temp_day_time;
	bool temp_blood_moon;
	int32_t temp_moon_phase;
	bool temp_eclipse;
	struct vector_2d dungeon;
	struct world_flags flags;
	uint8_t shadow_orb_count;
	int32_t altar_count;

	int32_t invasion_delay;
	int32_t invasion_size;
	int32_t invasion_size_start;
	int32_t invasion_type;
	double invasion_x;
	double slime_rain_time;
	uint8_t sundial_cooldown;
	int32_t rain_time;
	float max_rain;

	int32_t ore_tiers[3];

	uint8_t bg[8];

	float cloud_bg_active;

	int32_t num_clouds;

	float wind_speed;

	int32_t angler_quest;
	int32_t num_anglers;
	char **anglers;
	bool saved_angler;

	bool saved_stylist;
	bool saved_tax_collector;

	int16_t num_kill_counts;
	int32_t *kill_counts;

	bool fast_forward_time;

	struct binary_reader_context *reader;
	int _is_loaded;					/* Indicates if world_init has completed */
};

int world_new(TALLOC_CTX *parent, const char *world_path, struct world **out_world);
int world_init(struct world *world);

#endif /* _HAVE_WORLD_H */