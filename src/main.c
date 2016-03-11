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

#include <time.h>
#include <errno.h>
#include <uv.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "talloc/talloc.h"
#include "console.h"
#include "util.h"
#include "getopt.h"
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

static int __run_loop_init()
{
    return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;
	struct game_context *game;
	clock_t start;
	clock_t diff;

	printf("Upgraded Guacamole\n");

	parse_command_line(argc, argv);
	talloc_enable_leak_report_full();
	talloc_set_log_stderr();

	start = clock();

	printf("%s: initializing game context... ", __FUNCTION__);
	game_new(NULL, &game);
	printf("done.\n");

	printf("%s: loading world from %s... ", __FUNCTION__, options_worldPath);
	if (world_new(game, options_worldPath, &game->world) < 0
		|| world_init(game->world) < 0) {
		ret = -1;
		printf("World init failed: %d\n", ret);
		goto out;
	}
	
	diff = clock() - start;
	printf("done (%dms).\n", (int)(diff * 1000 / CLOCKS_PER_SEC));

    if (server_new(game, "0.0.0.0", 7777, game, &game->server) < 0) {
        _ERROR("Initializing TCP server for game context failed.");
        ret = -1;
        goto out;
    }
    
    server_start(game->server);
	
	console_init(game);

	printf("server started.\n");
	
    game_start_event_loop(game);
    	
out:
    talloc_free(game);

	talloc_report_full(NULL, stdout);

	return ret;
}

