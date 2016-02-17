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

struct tile {
	
};

struct world {
	unsigned long worldID;
	char *world_name;		/* World name */
	char *world_path;		/* Fully-qualified path to the world file */
	FILE *fp;				/* Pointer to the world file opened from world_path */
	int version;
	int num_positions;		/* Number of elements in the positions array */
	int *positions;			/* Array of position data */
	int num_important;		/* Number of elements in the importance array */
	unsigned *important;	/* Array of importance data */
	
};

#endif /* _HAVE_WORLD_H */