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

#include <stdio.h>
#include <string.h>

#include "world.h"

#define RELOGIC_MAGIC_NUMBER 27981915666277746

static int __world_read_file_metadata(struct world *world)
{
	int64_t magic;
	int64_t temp;
	int32_t revision;
	enum relogic_file_type type;

	if (binary_reader_read_int64(world->reader, &magic) < 0) {
		goto error;
	}

	if ((magic & 72057594037927935) != RELOGIC_MAGIC_NUMBER) {
		goto error;
	}

	type = (enum relogic_file_type) (magic >> 56 & 0xFF);

	if (type != relogic_file_type_world) {
		return -1;
	}

	if (binary_reader_read_int32(world->reader, &revision) < 0) {
		goto error;
	}

	if (binary_reader_read_int64(world->reader, &temp) < 0) {
		goto error;
	}

	return 0;

error:
	return -1;
}

static int __world_read_file_header(struct world *world)
{
	uint8_t flag1 = 0;
	uint8_t flag2 = 0x80;

	if (binary_reader_read_int32(world->reader, &world->version) < 0) {
		goto error;
	}

	if (__world_read_file_metadata(world) < 0) {
		printf("%s: world file metadata read failed.", __FUNCTION__);
	}

	if (binary_reader_read_int16(world->reader, &world->num_positions) < 0) {
		goto error;
	}

	world->positions = talloc_array(world, int32_t, world->num_positions);

	for (int i = 0; i < world->num_positions; i++) {
		if (binary_reader_read_int32(world->reader, &world->positions[i]) < 0) {
			goto error;
		}
		printf("%s: world->positions[%d] = %d\n", __FUNCTION__, i, world->positions[i]);
	}

	if (binary_reader_read_int16(world->reader, &world->num_important) < 0) {
		goto error;
	}

	world->important = talloc_array(world, int8_t, world->num_important);

	/*
	 * Note:
	 * 
	 * I have no idea what the fuck this routine does.
	 * 
	 * See WorldFile::LoadFileFormatHeader()
	 */
	for (int i = 0; i < world->num_important; i++) {

		if (flag2 != 0x80) {
			flag2 = flag2 << 1;
		} else {
			if (binary_reader_read_byte(world->reader, &flag1) < 0) {
				goto error;
			}

			flag2 = 1;
		}

		world->important[i] = (flag1 & flag2) == flag2;
	}

	return 0;

error:
	printf("Reading the world file failed: %d\n", -1);
	return -1;
}

static int __world_read_anglers(struct world *world)
{
	int ret = -1;

	if (binary_reader_read_int32(world->reader, &world->num_anglers) < 0) {
		printf("%s: binary reader error reading world->num_anglers\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	world->anglers = talloc_array(world, char *, (uint32_t)world->num_anglers);

	for(int i = world->num_anglers; i > 0; i--) {
		char *angler_finished;

		if (binary_reader_read_string(world->reader, &angler_finished) < 0) {
			ret = -1;
			goto out;
		}

		if ((world->anglers[i] = talloc_strdup(world->anglers, angler_finished)) == NULL) {
			printf("%s: out of memory copying angler text from world file.\n", __FUNCTION__);
			free(angler_finished);
			ret = -ENOMEM;
			goto out;
		}

		free(angler_finished);
	}

out:
	return ret;
}

static int __world_read_header(struct world *world)
{
	char *world_name;
	int ret;

	/*
	 * World header information is at position[0] in the positions
	 * table.
	 */
	fseek(world->reader->fp, world->positions[0], SEEK_SET);

	if (binary_reader_read_string(world->reader, &world_name) < 0) {
		printf("%s: binary reader error reading world->world_name\n", __FUNCTION__);
		return -1;
	}

	if ((world->world_name = talloc_strdup(world, world_name)) == NULL) {
		printf("%s: out of memory copying %d bytes for world name.\n", __FUNCTION__, (int)strlen(world_name));

		ret = -ENOMEM;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->worldID) < 0) {
		printf("%s: binary reader error reading world->worldID\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->left_world) < 0) {
		printf("%s: binary reader error reading world->left_world\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->right_world) < 0) {
		printf("%s: binary reader error reading world->right_world\n", __FUNCTION__);

		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->top_world) < 0) {
		printf("%s: binary reader error reading world->top_world\n", __FUNCTION__);

		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->bottom_world) < 0) {
		printf("%s: binary reader error reading world->bottom_world\n", __FUNCTION__);

		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->max_tiles_x) < 0) {
		printf("%s: binary reader error reading world->max_tiles_x\n", __FUNCTION__);

		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->max_tiles_y) < 0) {
		printf("%s: binary reader error reading world->max_tiles_y\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (world->version < 112) {
		world->expert_mode = false;
	} else {
		if (binary_reader_read_byte(world->reader, (uint8_t *)&world->expert_mode) < 0) {
			printf("%s: binary reader error reading world->expert_mode\n", __FUNCTION__);

			ret = -1;
			goto out;
		}
	}

	if (world->version >= 141) {
		if (binary_reader_read_int64(world->reader, &world->creation_time) < 0) {
			printf("%s: binary reader error reading world->creation_time\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (binary_reader_read_byte(world->reader, (uint8_t *)&world->moon_type) < 0) {
		printf("%s: binary reader error reading world->moon_type\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->tree_x[0]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_x[1]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_x[2]) < 0) {
		printf("%s: binary reader error reading world->tree_x\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->tree_style[0]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_style[1]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_style[2]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_style[3]) < 0) {
		printf("%s: binary reader error reading world->tree_style\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->cave_back_x[0]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_x[1]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_x[2]) < 0) {
		printf("%s: binary reader error reading world->cave_back_x\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->cave_back_style[0]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_style[1]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_style[2]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_style[3]) < 0) {
		printf("%s: binary reader error reading world->cave_back_style\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->ice_back_style) < 0
			|| binary_reader_read_int32(world->reader, &world->jungle_back_style) < 0
			|| binary_reader_read_int32(world->reader, &world->hell_back_style) < 0) {
		printf("%s: binary reader error reading world->back_styles\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->spawn_tile.x) < 0
			|| binary_reader_read_int32(world->reader, &world->spawn_tile.y) < 0) {
		ret = -1;
		goto out;
	}

	if (binary_reader_read_double(world->reader, &world->world_surface) < 0) {
		printf("%s: binary reader error reading world->world_surface\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_double(world->reader, &world->rock_layer) < 0) {
		printf("%s: binary reader error reading world->rock_layer\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_double(world->reader, &world->temp_time) < 0) {
		printf("%s: binary reader error reading world->temp_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->temp_day_time) < 0) {
		printf("%s: binary reader error reading world->temp_day_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->temp_moon_phase) < 0) {
		printf("%s: binary reader error reading world->temp_moon_phase\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->temp_blood_moon) < 0) {
		printf("%s: binary reader error reading world->temp_blood_moon\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->temp_eclipse) < 0) {
		printf("%s: binary reader error reading world->temp_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->dungeon.x) < 0
			|| binary_reader_read_int32(world->reader, &world->dungeon.y) < 0) {
		printf("%s: binary reader error reading world->dungeon\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.crimson) < 0) {
		printf("%s: binary reader error reading world->flags.crimson\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	printf("pos %li\n", ftell(world->reader->fp));

	if (binary_reader_read_boolean(world->reader, &world->flags.downed_boss_1) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_boss_2) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_boss_3) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_queen_bee) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_mech_1) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_mech_2) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_mech_3) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_mech_any) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_plant) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_golem) < 0) {
		printf("%s: binary reader error reading world->flags.downed_*\n", __FUNCTION__);
		ret = -1;
		goto out;
	}



	if (world->version >= 118) {
		if (binary_reader_read_boolean(world->reader, &world->flags.downed_slime_king) < 0) {
			printf("%s: binary reader error reading world->flags.downed_slime_king\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.saved_goblin) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.saved_wizard) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.saved_mech) < 0) {
		printf("%s: binary reader error reading world->flags.saved_*\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.downed_goblins) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_clowns) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_frost) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_pirates) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.shadow_orb_smashed) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.spawn_meteor) < 0) {
		printf("%s: binary reader error reading world->flags.downed_*\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_byte(world->reader, &world->shadow_orb_count) < 0) {
		printf("%s: binary reader error reading world->shadow_orb_count\n", __FUNCTION__);
		ret = -1;
		goto out;
	}


	if (binary_reader_read_int32(world->reader, &world->altar_count) < 0) {
		printf("%s: binary reader error reading world->altar_count\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.hard_mode) < 0) {
		printf("%s: binary reader error reading world->flags.hard_mode\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->invasion_delay) < 0) {
		printf("%s: binary reader error reading world->invasion_delay\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->invasion_size) < 0) {
		printf("%s: binary reader error reading world->invasion_size\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->invasion_type) < 0) {
		printf("%s: binary reader error reading world->invasion_type\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_double(world->reader, &world->invasion_x) < 0) {
		printf("%s: binary reader error reading world->invasion_x\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (world->version >= 118) {
		if (binary_reader_read_double(world->reader, &world->slime_rain_time) < 0) {
			printf("%s: binary reader error reading world->slime_rain_time\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (world->version >= 113) {
		if (binary_reader_read_byte(world->reader, &world->sundial_cooldown) < 0) {
			printf("%s: binary reader error reading world->sundial_cooldown\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.raining) < 0) {
		printf("%s: binary reader error reading world->flags.hard_mode\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->rain_time) < 0) {
		printf("%s: binary reader error reading world->rain_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->max_rain) < 0) {
		printf("%s: binary reader error reading world->max_rain\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->ore_tiers[0]) < 0
			|| binary_reader_read_int32(world->reader, &world->ore_tiers[1]) < 0
			|| binary_reader_read_int32(world->reader, &world->ore_tiers[2]) < 0) {
		printf("%s: binary reader error reading world->ore_tiers\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_byte(world->reader, &world->bg[0]) < 0
			|| binary_reader_read_byte(world->reader, &world->bg[1]) < 0
			|| binary_reader_read_byte(world->reader, &world->bg[2]) < 0
			|| binary_reader_read_byte(world->reader, &world->bg[3]) < 0
			|| binary_reader_read_byte(world->reader, &world->bg[4]) < 0
			|| binary_reader_read_byte(world->reader, &world->bg[5]) < 0
			|| binary_reader_read_byte(world->reader, &world->bg[6]) < 0
			|| binary_reader_read_byte(world->reader, &world->bg[7]) < 0) {
		printf("%s: binary reader error reading world->bg\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->cloud_bg_active) < 0) {
		printf("%s: binary reader error reading world->rain_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int16(world->reader, &world->num_clouds) < 0) {
		printf("%s: binary reader error reading world->num_clouds\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->wind_speed) < 0) {
		printf("%s: binary reader error reading world->wind_speed\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	__world_read_anglers(world);

	ret = 0;
out:
	free(world_name);

	return ret;
}

int world_init(struct world *world)
{
	int ret = 0;

	if ((ret = binary_reader_open(world->reader)) < 0) {
		printf("Opening world file failed: %d\n", ret);
		ret = -1;
		goto out;
	}

	if ((ret = __world_read_file_header(world)) < 0) {
		printf("Reading world file headers failed: %d\n", ret);
	}

	__world_read_header(world);

out:
	return ret;
}

int world_new(TALLOC_CTX *parent_context, const char *world_path,
			  struct world **out_world)
{
	int ret = 0;
	TALLOC_CTX *tempContext;
	struct world *world;

	if ((tempContext = talloc_new(NULL)) == NULL) {
		ret = -ENOMEM;
		goto failed;
	}

	world = talloc_zero(tempContext, struct world);

	if ((world->world_path = talloc_strdup(world, world_path)) == NULL) {
		ret = -ENOMEM;
		goto failed;
	}

	if (binary_reader_new(world, world->world_path, &world->reader) < 0) {
		printf("world - could not allocate a binary reader for the world.");
		ret = -ENOMEM;
		goto failed;
	}

	*out_world = talloc_steal(parent_context, world);
	ret = 0;

failed:
	talloc_free(tempContext);
	return ret;
}