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

static int __world_read_file_header(struct world *world)
{
	if (binary_reader_read_int32(world->reader, &world->version) < 0) {
		goto error;
	}
	
	if (binary_reader_read_int16(world->reader, &world->num_positions) < 0) {
		goto error;
	}

	world->positions = talloc_array(world, int, world->num_positions);

	for (int i = 0; i < world->num_positions; i++) {
		if (binary_reader_read_int32(world->reader, &world->positions[i]) < 0) {
			goto error;
		}
	}

	if (binary_reader_read_int16(world->reader, &world->num_important) < 0) {
		goto error;
	}

	world->important = talloc_array(world, uint8_t, &world->num_important);

	for (int i = 0; i < world->num_important; i++) {
		if (binary_reader_read_boolean(world->reader, &world->important[i]) < 0) {
			goto error;
		}
	}

	return 0;

error:
	printf("Reading the world file failed: %d\n", -1);
	return -1;
}

int world_init(struct world *world)
{
	int ret = 0;

	if ((ret = binary_reader_open(world->reader)) < 0) {
		printf("Opening world file failed: %d\n", ret);
		ret = -1;
		goto out;
	}

	ret = __world_read_file_header(world);

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