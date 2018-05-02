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

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <uv.h>

#ifndef _WIN32
#include <unistd.h>
#else
//#define PROTOBUF_USE_DLLS
#endif

#include "console.h"
#include "game.h"
#include "getopt.h"
#include "server.h"
#include "talloc/talloc.h"
#include "util.h"
#include "world.h"

#define OPTIONS "sw:"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup paper-tiger The Paper Tiger API
 *
 * Paper Tiger is an open-source Terraria server that aims to implement
 * official Terraria clients in a more efficient and cross-platform
 * manner.
 */

static const char *options_worldPath = NULL;
static bool options_console = true;

static int
parse_command_line(int argc, char **argv)
{
	int c;

	opterr = 0;

	while ((c = getopt(argc, argv, OPTIONS)) != -1) {
		switch (c) {
		case 's':
			options_console = false;
			break;
		case 'w':
			options_worldPath = optarg;
			break;
		default:
			break;
		}
	}

	return 0;
}

static void
__close_handle(uv_handle_t *handle, void *context)
{
	uv_close(handle, NULL);
}

int
main(int argc, char **argv)
{
	int ret = 0;
	struct game *game;
	struct world world;
	clock_t start;
	clock_t diff;
	int loop_close_result = 0;

	printf("Paper Tiger Terraria Server\n");
	printf(" by Tyler W. <tyler@tw.id.au>\n\n");

	parse_command_line(argc, argv);
	// talloc_enable_leak_report_full();
	// talloc_set_log_stderr();

	start = clock();

	printf("%s: initializing game context... ", __FUNCTION__);

	if (game_new(NULL, &game) < 0) {
		printf("failed.\n");
		_ERROR("%s: game initialization failed.  The process cannot continue.\n", __FUNCTION__);
		return 2;
	}

	printf("done.\n");

	printf("%s: loading world from %s... ", __FUNCTION__, options_worldPath);

	world.game = game;

	if (world_init(game, &world, options_worldPath) < 0) {
		ret = -1;
		printf("World init failed: %d\n", ret);
		goto out;
	}

	game->world = world;

	if (uv_timer_init(&game->event_loop, &world.section_compress_worker) < 0) {
		_ERROR("%s: initializing section compress worker failed.\n", __FUNCTION__);
		goto out;
	}

	diff = clock() - start;
	printf("done (%dms).\n", (int)(diff * 1000 / CLOCKS_PER_SEC));

	if (server_new(game, "0.0.0.0", 7777, game, &game->server) < 0) {
		_ERROR("Initializing TCP server for game context failed.\n");
		ret = -1;
		goto out;
	}

	if (server_start(game->server) < 0) {
		_ERROR("Starting server for game context failed.\n");
		ret = -1;
		goto out;
	}

	game_update_loop_init(game);

	printf("\nStarted successfully on %s:%d\n", game->server->listen_address, game->server->port);

	printf(" * %s (%dx%d)\n", world.world_name, world.max_tiles_x, world.max_tiles_y);

	printf(" * Expert: %s, Crimson: %s\n", game->world.expert_mode ? "Yes" : "No",
		   game->world.flags.crimson ? "Yes" : "No");

	printf("\n");

	if (options_console == true && console_init(&game->console, game) < 0) {
		_ERROR("Initializing console failed.\n");
		ret = -1;
		goto out;
	}

	game_start_event_loop(game);

	uv_walk(&game->event_loop, __close_handle, NULL);
	uv_run(&game->event_loop, UV_RUN_DEFAULT);

	if (uv_loop_close(&game->event_loop) == UV_EBUSY) {
		printf("%s: warning: event loop has not signalled a close.", __FUNCTION__);
	}

out:
	talloc_free(game);
	return ret;
}

#ifdef __cplusplus
}
#endif