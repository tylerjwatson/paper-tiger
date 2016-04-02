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

#define WORLD_SECTION_WIDTH 200
#define WORLD_SECTION_HEIGHT 150

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

#include "talloc/talloc.h"

#include "vector_2d.h"

struct game;
struct rect;
struct tile;
struct binary_reader_context;

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
	bool saved_angler;
	bool saved_stylist;
	bool saved_tax_collector;
	bool shadow_orb_smashed;
	bool spawn_meteor;
	bool hard_mode;
	bool raining;
	bool fast_forward_time;
};

enum relogic_file_type {
	relogic_file_type_none,
	relogic_file_type_map,
	relogic_file_type_world,
	relogic_file_type_player
};

/**
 * Structure which describes a Terraria world.  The structure contains a pointer to
 * all the tiles and everything the world needs to operate correctly.
 */
struct world {
	struct game *game;
	/**
	 * Internal unique world identifier.
	 */
	int32_t worldID;
	
	/** 
	 * World name
	 */
	char *world_name;		
	
	/** Fully-qualified path to the world file */
	char *world_path;
	
	/**
	 * World version as recorded in the world file.
	 */
	int version;
	
	/** Number of elements in the positions array */
	uint16_t num_positions;
	
	/** Array of position data */
	int32_t *positions;
	
	/** Number of elements in the importance array */
	uint16_t num_important;
	
	/** Array of importance data */
	int8_t *important;

	float left_world;
	float right_world;
	float top_world;
	float bottom_world;

	/** 
	 * The number of tiles in the world's X axis.
	 */
	uint32_t max_tiles_x;		
	
	/** 
	 * The number of tiles i n the world's Y axis.
	 */
	uint32_t max_tiles_y;		

	/*
	 * DateTime stamp of when the world file was created
	 */
	int64_t creation_time;

	/*
	 * Indicates whether the world has expert mode enabled
	 */
	bool expert_mode;
	
	int8_t moon_type;
	int32_t tree_x[3];
	int32_t tree_style[4];
	int32_t cave_back_x[3];
	int32_t cave_back_style[4];
	int32_t ice_back_style;
	int32_t jungle_back_style;
	int32_t hell_back_style;
	
	/*
	 * Coordinates for the spawn point in the map.
	 */
	struct vector_2d spawn_tile;
	
	double world_surface;
	double rock_layer;
	double temp_time;
	bool temp_day_time;
	bool temp_blood_moon;
	int32_t temp_moon_phase;
	bool temp_eclipse;
	
	/*
	 * Contains the coordinates of the map's dungeon.
	 */
	struct vector_2d dungeon;
	
	/*
	 * 	A list of flags relating to events that have happened in the
	 * world.
	 */
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

	int16_t num_clouds;

	float wind_speed;

	int32_t cultist_delay;
	int32_t angler_quest;
	
	/**
	 * The number of angler string elements in the anglers array.
	 */
	int32_t num_anglers;
	
	/**
	 * Angler text array
	 */
	char **anglers;
	
	bool saved_angler;

	bool saved_stylist;
	bool saved_tax_collector;

	/**
	 * Number of elements in the kill counts table
	 */
	int16_t num_kill_counts;
	
	/**
	 * Array of kill counts
	 */
	int32_t *kill_counts;

	/**
	 * Indicates whether world time runs at normal speed or fast speed.
	 */
	bool fast_forward_time;

	/**
	 * 2D array of tile structures which make up the world.
	 */
	struct tile **tiles;

	/**
	 * Reference to a binary reader which is used to read data from
	 * the world file specified on the command line.
	 */
	struct binary_reader_context *reader;
	
	/**
	 * Indicates if world_init has completed
	 */
	int _is_loaded; 
};

int world_new(TALLOC_CTX *parent_context, const struct game *game, const char *world_path,
			  struct world **out_world);

int world_init(struct world *world);

struct tile *world_tile_at(struct world *world, const uint32_t x, const uint32_t y);

struct vector_2d world_tile_section(int x, int y);

int world_pack_tile_section(TALLOC_CTX *context, struct world *world, struct rect rect,
							char *tile_buffer, int *out_buf_len);

#ifdef __cplusplus
}
#endif