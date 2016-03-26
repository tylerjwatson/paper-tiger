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

#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "world.h"
#include "util.h"
#include "binary_writer.h"

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
		_ERROR("%s: world file metadata read failed.", __FUNCTION__);
	}

	if (binary_reader_read_uint16(world->reader, &world->num_positions) < 0) {
		goto error;
	}

	world->positions = talloc_array(world, int32_t, world->num_positions);

	for (int i = 0; i < world->num_positions; i++) {
		if (binary_reader_read_int32(world->reader, &world->positions[i]) < 0) {
			goto error;
		}
	}

	if (binary_reader_read_uint16(world->reader, &world->num_important) < 0) {
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
	_ERROR("Reading the world file failed: %d\n", -1);
	return -1;
}

static int __world_read_anglers(struct world *world)
{
	int ret = -1;

	if (binary_reader_read_int32(world->reader, &world->num_anglers) < 0) {
		_ERROR("%s: binary reader error reading world->num_anglers\n", __FUNCTION__);
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
			_ERROR("%s: out of memory copying angler text from world file.\n", __FUNCTION__);
			free(angler_finished);
			ret = -ENOMEM;
			goto out;
		}

		free(angler_finished);
	}

out:
	return ret;
}

static int __world_read_npc_killcounts(struct world *world)
{
	int ret = -1;

	if (binary_reader_read_int16(world->reader, &world->num_kill_counts) < 0) {
		_ERROR("%s: binary reader error reading world->num_kill_counts\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if ((world->kill_counts = talloc_array(world, int32_t, (uint32_t)world->num_kill_counts)) == NULL) {
		_ERROR("%s: out of memory allocating world->kill_counts\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	for (int i = 0; i < world->num_kill_counts; i++) {
		if (binary_reader_read_int32(world->reader, &world->kill_counts[i]) < 0) {
			_ERROR("%s: binary reader error reading world->num_kill_counts\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	ret = 0;
out:
	return ret;
}

static int __world_load_tile(struct world *world, uint32_t x, uint32_t y, uint16_t *out_tile_copies)
{
	int ret = -1;
	int liquid_type = 0;
	uint16_t tile_copies = 0;
	uint8_t tile_flags_1 = 0, tile_wire_flags = 0, tile_colour_flags = 0;
	struct tile *tile = world_tile_at(world, x, y);

	tile->type = 0;

	//_ERROR("tile %d,%d @ pos %ld\n", x, y, ftell(world->reader->fp));

	/*
	 * num6 = tile_flags_1
	 * num5 = tile_wire_flags
	 * num4 = tile_colour_flags
	 */

	if (binary_reader_read_byte(world->reader, &tile_flags_1) < 0) {
		_ERROR("%s: binary reader error reading tile_flags_1", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if ((tile_flags_1 & 1) == 1) {
		if (binary_reader_read_byte(world->reader, &tile_wire_flags) < 0) {
			_ERROR("%s: binary reader error reading tile_wire_flags", __FUNCTION__);
			ret = -1;
			goto out;
		}

		if ((tile_wire_flags & 1) == 1) {
			if (binary_reader_read_byte(world->reader, &tile_colour_flags) < 0) {
				_ERROR("%s: binary reader error reading tile_colour_flags", __FUNCTION__);
				ret = -1;
				goto out;
			}

		}
	}

	if ((tile_flags_1 & WORLD_FILE_TILE_ACTIVE) == WORLD_FILE_TILE_ACTIVE) {
		tile_set_active(tile, true);

		if ((tile_flags_1 & WORLD_FILE_TYPE_SHORT) == WORLD_FILE_TYPE_SHORT) {
			ret = binary_reader_read_uint16(world->reader, &tile->type);
		} else {
			ret = binary_reader_read_byte(world->reader, (uint8_t *)&tile->type);
		}

		if (ret < 0) {
			goto out;
		}

		if (world->important[tile->type] == false) {
			tile->frame_x = tile->frame_y = -1;
		} else {
			if (binary_reader_read_int16(world->reader, &tile->frame_x) < 0
					|| binary_reader_read_int16(world->reader, &tile->frame_y) < 0) {
				_ERROR("%s: binary reader error reading tile->frame_[xy]", __FUNCTION__);
				ret = -1;
				goto out;
			}

			if (tile->type == 144) {
				tile->frame_y = 0;
			}
		}

		/*
		 * ??????
		 */
		if ((tile_colour_flags & WORLD_FILE_TILE_COLOUR) == WORLD_FILE_TILE_COLOUR) {
			uint8_t colour;

			if (binary_reader_read_byte(world->reader, &colour) < 0) {
				_ERROR("%s: binary reader error reading tile colour", __FUNCTION__);
				ret = -1;
				goto out;
			}

			tile_set_colour(tile, colour);
		}
	}

	if ((tile_flags_1 & WORLD_FILE_TILE_IS_WALL) == WORLD_FILE_TILE_IS_WALL) {
		if (binary_reader_read_byte(world->reader, &tile->wall) < 0) {
			_ERROR("%s: binary reader error reading tile->wall", __FUNCTION__);
			ret = -1;
			goto out;
		}

		if ((tile_colour_flags & WORLD_FILE_WALL_COLOUR) == WORLD_FILE_WALL_COLOUR) {
			uint8_t wall_colour;

			if (binary_reader_read_byte(world->reader, &wall_colour) < 0) {
				_ERROR("%s: binary reader error reading tile->wall_colour", __FUNCTION__);
				ret = -1;
				goto out;
			}

			tile_set_wall_colour(tile, wall_colour);
		}
	}

	if ((liquid_type = (tile_flags_1 & 24) >> 3) != 0) {
		if (binary_reader_read_byte(world->reader, &tile->liquid) < 0) {
			_ERROR("%s: binary reader error reading tile->liquid", __FUNCTION__);
			ret = -1;
			goto out;
		}

		if (liquid_type != 2) {
			tile_set_honey(tile, true);
		} else {
			tile_set_lava(tile, true);
		}
	}

	if (tile_wire_flags > 1) {
		tile_set_wire(tile, (tile_wire_flags & TILE_WIRE_1) == TILE_WIRE_1);
		tile_set_wire_2(tile, (tile_wire_flags & TILE_WIRE_2) == TILE_WIRE_2);
		tile_set_wire_3(tile, (tile_wire_flags & TILE_WIRE_3) == TILE_WIRE_3);
	}

	//TODO: tile slope

	tile_set_actuator(tile, (tile_colour_flags & WORLD_FILE_TILE_COLOUR_ACTUATOR) == WORLD_FILE_TILE_COLOUR_ACTUATOR);
	tile_set_inactive(tile, (tile_colour_flags & WORLD_FILE_TILE_COLOUR_INACTIVE) == WORLD_FILE_TILE_COLOUR_INACTIVE);

	tile_copies = (uint16_t)(tile_flags_1 & 192) >> 6;

	if (tile_copies > 0) {
		if (tile_copies != 1) {
			if (binary_reader_read_uint16(world->reader, &tile_copies) < 0) {
				_ERROR("%s: binary error reading tile_copies from tile", __FUNCTION__);
			}
		} else {
			if (binary_reader_read_byte(world->reader, (uint8_t *) &tile_copies) < 0) {
				_ERROR("%s: binary error reading tile_copies from tile", __FUNCTION__);
			}

		}
	}

	//TODO: some WorldGen.TileCounts bullshit

	*out_tile_copies = tile_copies;
	ret = 0;
out:
	return ret;
}

static int __world_read_tile(struct world *world)
{
	int ret = 0;


	if (fseek(world->reader->fp, world->positions[1], SEEK_SET) < 0) {
		_ERROR("%s: could not seek to position %d in world file.\n", __FUNCTION__, world->positions[1]);
		ret = -1;
		goto out;
	}

	if (tile_heap_new(world, (uint32_t)world->max_tiles_x, (uint32_t)world->max_tiles_y, &world->tiles) < 0) {
		_ERROR("%s: cannot allocate the tile heap.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	for (unsigned int x = 0; x < world->max_tiles_x; x++) {
		for (unsigned int y = 0; y < world->max_tiles_y; y++) {
			uint16_t num_copies = 0;
			const struct tile *src_tile;

			if (__world_load_tile(world, x, y, &num_copies) < 0) {
				_ERROR("%s: tile error in %d,%d.\n", __FUNCTION__, x, y);
				ret = -1;
				goto out;
			}

			src_tile = world_tile_at(world, x, y);

			for(unsigned y_copy = 1; y_copy <= num_copies; y_copy++) {
				tile_copy(src_tile, world_tile_at(world, x, y + y_copy));
			}

			y += num_copies;
		}

		//return -2;
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
		_ERROR("%s: binary reader error reading world->world_name\n", __FUNCTION__);
		return -1;
	}

	if ((world->world_name = talloc_strdup(world, world_name)) == NULL) {
		_ERROR("%s: out of memory copying %d bytes for world name.\n", __FUNCTION__, (int)strlen(world_name));

		ret = -ENOMEM;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->worldID) < 0) {
		_ERROR("%s: binary reader error reading world->worldID\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->left_world) < 0) {
		_ERROR("%s: binary reader error reading world->left_world\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->right_world) < 0) {
		_ERROR("%s: binary reader error reading world->right_world\n", __FUNCTION__);

		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->top_world) < 0) {
		_ERROR("%s: binary reader error reading world->top_world\n", __FUNCTION__);

		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->bottom_world) < 0) {
		_ERROR("%s: binary reader error reading world->bottom_world\n", __FUNCTION__);

		ret = -1;
		goto out;
	}

	if (binary_reader_read_uint32(world->reader, &world->max_tiles_y) < 0) {
		_ERROR("%s: binary reader error reading world->max_tiles_y\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_uint32(world->reader, &world->max_tiles_x) < 0) {
		_ERROR("%s: binary reader error reading world->max_tiles_x\n", __FUNCTION__);

		ret = -1;
		goto out;
	}

	if (world->version < 112) {
		world->expert_mode = false;
	} else {
		if (binary_reader_read_byte(world->reader, (uint8_t *)&world->expert_mode) < 0) {
			_ERROR("%s: binary reader error reading world->expert_mode\n", __FUNCTION__);

			ret = -1;
			goto out;
		}
	}

	if (world->version >= 141) {
		if (binary_reader_read_int64(world->reader, &world->creation_time) < 0) {
			_ERROR("%s: binary reader error reading world->creation_time\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (binary_reader_read_byte(world->reader, (uint8_t *)&world->moon_type) < 0) {
		_ERROR("%s: binary reader error reading world->moon_type\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->tree_x[0]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_x[1]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_x[2]) < 0) {
		_ERROR("%s: binary reader error reading world->tree_x\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->tree_style[0]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_style[1]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_style[2]) < 0
			|| binary_reader_read_int32(world->reader, &world->tree_style[3]) < 0) {
		_ERROR("%s: binary reader error reading world->tree_style\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->cave_back_x[0]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_x[1]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_x[2]) < 0) {
		_ERROR("%s: binary reader error reading world->cave_back_x\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->cave_back_style[0]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_style[1]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_style[2]) < 0
		|| binary_reader_read_int32(world->reader, &world->cave_back_style[3]) < 0) {
		_ERROR("%s: binary reader error reading world->cave_back_style\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->ice_back_style) < 0
			|| binary_reader_read_int32(world->reader, &world->jungle_back_style) < 0
			|| binary_reader_read_int32(world->reader, &world->hell_back_style) < 0) {
		_ERROR("%s: binary reader error reading world->back_styles\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->spawn_tile.x) < 0
			|| binary_reader_read_int32(world->reader, &world->spawn_tile.y) < 0) {
		ret = -1;
		goto out;
	}

	if (binary_reader_read_double(world->reader, &world->world_surface) < 0) {
		_ERROR("%s: binary reader error reading world->world_surface\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_double(world->reader, &world->rock_layer) < 0) {
		_ERROR("%s: binary reader error reading world->rock_layer\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_double(world->reader, &world->temp_time) < 0) {
		_ERROR("%s: binary reader error reading world->temp_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->temp_day_time) < 0) {
		_ERROR("%s: binary reader error reading world->temp_day_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->temp_moon_phase) < 0) {
		_ERROR("%s: binary reader error reading world->temp_moon_phase\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->temp_blood_moon) < 0) {
		_ERROR("%s: binary reader error reading world->temp_blood_moon\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->temp_eclipse) < 0) {
		_ERROR("%s: binary reader error reading world->temp_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->dungeon.x) < 0
			|| binary_reader_read_int32(world->reader, &world->dungeon.y) < 0) {
		_ERROR("%s: binary reader error reading world->dungeon\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.crimson) < 0) {
		_ERROR("%s: binary reader error reading world->flags.crimson\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	//_ERROR("pos %li\n", ftell(world->reader->fp));

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
		_ERROR("%s: binary reader error reading world->flags.downed_*\n", __FUNCTION__);
		ret = -1;
		goto out;
	}



	if (world->version >= 118) {
		if (binary_reader_read_boolean(world->reader, &world->flags.downed_slime_king) < 0) {
			_ERROR("%s: binary reader error reading world->flags.downed_slime_king\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.saved_goblin) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.saved_wizard) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.saved_mech) < 0) {
		_ERROR("%s: binary reader error reading world->flags.saved_*\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.downed_goblins) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_clowns) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_frost) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_pirates) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.shadow_orb_smashed) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.spawn_meteor) < 0) {
		_ERROR("%s: binary reader error reading world->flags.downed_*\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_byte(world->reader, &world->shadow_orb_count) < 0) {
		_ERROR("%s: binary reader error reading world->shadow_orb_count\n", __FUNCTION__);
		ret = -1;
		goto out;
	}


	if (binary_reader_read_int32(world->reader, &world->altar_count) < 0) {
		_ERROR("%s: binary reader error reading world->altar_count\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.hard_mode) < 0) {
		_ERROR("%s: binary reader error reading world->flags.hard_mode\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->invasion_delay) < 0) {
		_ERROR("%s: binary reader error reading world->invasion_delay\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->invasion_size) < 0) {
		_ERROR("%s: binary reader error reading world->invasion_size\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->invasion_type) < 0) {
		_ERROR("%s: binary reader error reading world->invasion_type\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_double(world->reader, &world->invasion_x) < 0) {
		_ERROR("%s: binary reader error reading world->invasion_x\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (world->version >= 118) {
		if (binary_reader_read_double(world->reader, &world->slime_rain_time) < 0) {
			_ERROR("%s: binary reader error reading world->slime_rain_time\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (world->version >= 113) {
		if (binary_reader_read_byte(world->reader, &world->sundial_cooldown) < 0) {
			_ERROR("%s: binary reader error reading world->sundial_cooldown\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.raining) < 0) {
		_ERROR("%s: binary reader error reading world->flags.hard_mode\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->rain_time) < 0) {
		_ERROR("%s: binary reader error reading world->rain_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->max_rain) < 0) {
		_ERROR("%s: binary reader error reading world->max_rain\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->ore_tiers[0]) < 0
			|| binary_reader_read_int32(world->reader, &world->ore_tiers[1]) < 0
			|| binary_reader_read_int32(world->reader, &world->ore_tiers[2]) < 0) {
		_ERROR("%s: binary reader error reading world->ore_tiers\n", __FUNCTION__);
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
		_ERROR("%s: binary reader error reading world->bg\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->cloud_bg_active) < 0) {
		_ERROR("%s: binary reader error reading world->rain_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int16(world->reader, &world->num_clouds) < 0) {
		_ERROR("%s: binary reader error reading world->num_clouds\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (binary_reader_read_single(world->reader, &world->wind_speed) < 0) {
		_ERROR("%s: binary reader error reading world->wind_speed\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (world->version < 95) {
		ret = 0;
		goto out;
	}

	__world_read_anglers(world);

	if (world->version < 99) {
		ret = 0;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.saved_angler) < 0) {
		_ERROR("%s: binary reader error reading world->flags.saved_angler\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (world->version < 101) {
		ret = 0;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->angler_quest) < 0) {
		_ERROR("%s: binary reader error reading world->angler_quest\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (world->version < 104) {
		ret = 0;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.saved_stylist) < 0) {
		_ERROR("%s: binary reader error reading world->flags.saved_stylist\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (world->version >= 129) {
		if (binary_reader_read_boolean(world->reader, &world->flags.saved_tax_collector) < 0) {
			_ERROR("%s: binary reader error reading world->flags.saved_tax_collector\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (world->version >= 107) {
		if (binary_reader_read_int32(world->reader, &world->invasion_size_start) < 0) {
			_ERROR("%s: binary reader error reading world->invasion_size_start\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (world->version >= 108) {
		if (binary_reader_read_int32(world->reader, &world->cultist_delay) < 0) {
			_ERROR("%s: binary reader error reading world->cultist_delay\n", __FUNCTION__);
			ret = -1;
			goto out;
		}
	}

	if (world->version < 109) {
		ret = 0;
		goto out;
	}

	if (__world_read_npc_killcounts(world) < 0) {
		ret = -1;
		goto out;
	}

	if (world->version < 128) {
		ret = 0;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.fast_forward_time) < 0) {
		_ERROR("%s: binary reader error reading world->flags.fast_forward_time\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (world->version < 131) {
		ret = 0;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.downed_fishron) < 0
		|| binary_reader_read_boolean(world->reader, &world->flags.downed_martians) < 0
		|| binary_reader_read_boolean(world->reader, &world->flags.downed_ancient_cultist) < 0
		|| binary_reader_read_boolean(world->reader, &world->flags.downed_moonlord) < 0
		|| binary_reader_read_boolean(world->reader, &world->flags.downed_halloween_king) < 0
		|| binary_reader_read_boolean(world->reader, &world->flags.downed_halloween_tree) < 0
		|| binary_reader_read_boolean(world->reader, &world->flags.downed_christmas_ice_queen) < 0
		|| binary_reader_read_boolean(world->reader, &world->flags.downed_christmas_santank) < 0
		|| binary_reader_read_boolean(world->reader, &world->flags.downed_christmas_tree) < 0) {
		_ERROR("%s: binary reader error reading world->flags.downed_*\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (world->version < 140) {
		ret = 0;
		goto out;
	}

	if (binary_reader_read_boolean(world->reader, &world->flags.downed_tower_solar) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_tower_vortex) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_tower_nebula) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.downed_tower_stardust) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.active_tower_solar) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.active_tower_vortex) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.active_tower_nebula) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.active_tower_stardust) < 0
			|| binary_reader_read_boolean(world->reader, &world->flags.lunar_apocalypse_up) < 0) {
		_ERROR("%s: binary reader error reading world->flags.downed_fishron\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	ret = 0;
out:
	free(world_name);

	return ret;
}

int world_init(struct world *world)
{
	int ret = 0;

	if ((ret = binary_reader_open(world->reader)) < 0) {
		_ERROR("Opening world file failed: %d\n", ret);
		ret = -1;
		goto out;
	}

	if ((ret = __world_read_file_header(world)) < 0) {
		_ERROR("Reading world file headers failed: %d\n", ret);
		goto out;
	}

	if ((ret = __world_read_header(world)) < 0) {
		_ERROR("Reading world headers failed: %d\n", ret);
		goto out;
	}

	if ((ret = __world_read_tile(world)) < 0) {
		_ERROR("Reading world headers failed: %d\n", ret);
	}

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
		_ERROR("world - could not allocate a binary reader for the world.");
		ret = -ENOMEM;
		goto failed;
	}

	*out_world = talloc_steal(parent_context, world);
	ret = 0;

failed:
	talloc_free(tempContext);
	return ret;
}

struct tile *world_tile_at(struct world *world, const uint32_t x, const uint32_t y)
{
	return &world->tiles[x][y];
}

int world_compress_tile_section(struct world *world, unsigned start_x, unsigned start_y, unsigned w, unsigned h, char *buffer, int *buf_len)
{
	struct tile *tile;
	int staging_len = 0, pos = 0, ret = -1;
	uint8_t header_1, header_2, header_3;

	char staging_buffer[13];
	char tile_buffer[262144];

	for (unsigned y = start_y; y < start_y + w; y++)
	for (unsigned x = start_x; x < start_x + h; x++) {
		tile = world_tile_at(world, x, y);
		staging_len = tile_pack(tile, staging_buffer, &header_1, &header_2, &header_3);
		pos += binary_writer_write_value(tile_buffer + pos, header_1);

		if ((header_1 & 1) == 1) {
			pos += binary_writer_write_value(tile_buffer + pos, header_2);

			if ((header_2 & 1) == 1) {
				pos += binary_writer_write_value(tile_buffer + pos, header_3);
			}
		}

		memcpy(tile_buffer + pos, staging_buffer, staging_len);
		pos += staging_len;
	}

	if (compress(buffer, buf_len, tile_buffer, pos) != Z_OK) {
		_ERROR("%s: compression error writing tile section %d,%d.\n", __FUNCTION__, start_x, start_y);
		ret = -1;
	}

	return ret;
}