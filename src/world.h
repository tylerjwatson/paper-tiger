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

enum relogic_file_type {
	relogic_file_type_none,
	relogic_file_type_map,
	relogic_file_type_world,
	relogic_file_type_player
};

struct world {
	unsigned long worldID;
	char *world_name;		/* World name */
	char *world_path;		/* Fully-qualified path to the world file */
	int version;
	int16_t num_positions;		/* Number of elements in the positions array */
	int32_t *positions;			/* Array of position data */
	int16_t num_important;		/* Number of elements in the importance array */
	int8_t *important;		/* Array of importance data */

	float left_world;
	float right_world;
	float top_world;
	float bottom_world;
	
	struct binary_reader_context *reader;

	int _is_loaded;					/* Indicates if world_init has completed */
};

int world_new(TALLOC_CTX *parent, const char *world_path, struct world **out_world);
int world_init(struct world *world);

#endif /* _HAVE_WORLD_H */