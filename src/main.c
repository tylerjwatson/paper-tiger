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

#include "talloc/talloc.h"
#include <errno.h>
#include "getopt.h"
#include <time.h>
#include "game.h"
#include "world.h"

#define GUAC_OPTIONS "w:"

static const char *options_worldPath = NULL;

static int parse_command_line(int argc, char **argv)
{
	int c;

	opterr = 0;

	while ((c = getopt(argc, argv, GUAC_OPTIONS)) != -1) {
		switch (c) {
		case 'w':
			options_worldPath = optarg;
			break;
		default:
			break;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;
	struct world *world;

	printf("Upgraded Guacamole\n");

	parse_command_line(argc, argv);

	clock_t start = clock(), diff;

	if (world_new(NULL, options_worldPath, &world) < 0
		|| world_init(world) < 0) {
		ret = -1;
		printf("World init failed: %d\n", ret);
		goto out;
	}
	diff = clock() - start;

	printf("%s: world loaded in %dms\n", __FUNCTION__, diff * 1000 / CLOCKS_PER_SEC);

//	talloc_report_full(world, stderr);
	system("pause");
	talloc_free(world);
out:
	return ret;
}

