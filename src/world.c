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

	type = (enum relogic_file_type)(magic >> 56 & 0xFF);

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

	world->important = talloc_array(world, uint8_t, world->num_important);

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
		return -1;
	}
	
	world->world_name = talloc_strdup(world, world_name);
	
	if (binary_reader_read_int32(world->reader, &world->worldID) < 0) {
		ret = -1;
		goto out;
	}

	if (binary_reader_read_int32(world->reader, &world->worldID) < 0) {
	}
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