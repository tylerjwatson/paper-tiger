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

#ifndef _WIN32
#include <unistd.h>
#else

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <WinSock2.h>
#include <Windows.h>
#endif

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <uv.h>
#include "talloc/talloc.h"

#include "game.h"
#include "getopt.h"
#include "game_properties.h"

#include "log.h"

#define OPTIONS "p:P:a:sw:"

#ifdef __cplusplus
extern "C" {
#endif

static uv_tty_t console;

/**
 * @defgroup paper-tiger The Paper Tiger API
 *
 * Paper Tiger is an open-source Terraria server that aims to implement
 * official Terraria clients in a more efficient and cross-platform
 * manner.
 */


static void
__close_handle(uv_handle_t *handle, void *context)
{
	uv_close(handle, NULL);
}

int
main(int argc, char **argv)
{
	const uv_loop_t *loop = uv_default_loop();

	int ret = 0;

	ptGame game;

	clock_t start, diff;
	int loop_close_result = 0;

	opterr = 0;

	start = clock();


	uv_tty_t out;

	uv_tty_init((uv_loop_t *)loop, &console, 0, 1);
	uv_tty_set_mode(&console, UV_TTY_MODE_NORMAL);
	uv_tty_init((uv_loop_t *)loop, &out, 1, 1);
	uv_tty_set_mode(&console, UV_TTY_MODE_NORMAL);
	uv_tty_init((uv_loop_t *)loop, &out, 2, 1);
	uv_tty_set_mode(&console, UV_TTY_MODE_NORMAL);


	log_info("Paper Tiger Terraria Server by Tyler W. <tyler@tw.id.au>");

	//while ((c = getopt(argc, argv, OPTIONS)) != -1) {
	//	switch (c) {
 //       case 'a': {
	//		gameProperties.listenAddr = optarg;
	//	} break;
 //       case 'P':
	//		gameProperties.listenPort = atoi(optarg);
	//		break;
	//	case 'p':
	//		gameProperties.maxPlayers = atoi(optarg);
	//		break;
	//	case 's':
	//		gameProperties.enableConsole = false;
	//		break;
	//	case 'w':
	//		gameProperties.worldFilePath = optarg;
	//		break;
	//	default:
	//		break;
	//	}
	//}

	if ((ret = ptGameInitialize(&game, loop)) < 0) {

		log_fatal("Game initialization failed.");
		return ret;
	}

    diff = clock() - start;

	log_debug("game context %llu bytes in %dms.\n", sizeof(game), diff);

	log_info("Game is running");
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	// printf("%s: loading world from %s... ", __FUNCTION__, options_worldPath);

	// world.game = game;

	// if (world_init(game, &world, options_worldPath) < 0) {
	//	ret = -1;
	//	printf("World init failed: %d\n", ret);
	//	goto out;
	//}

	// game->world = &world; //TODO: Fix this obvious segfault

	// if (uv_timer_init(&game->eventLoop, &world.section_compress_worker) < 0)
	// { 	_ERROR("%s: initializing section compress worker failed.\n",
	//__FUNCTION__); 	goto out;
	//}

	// diff = clock() - start;
	// printf("done (%dms).\n", (int)(diff * 1000 / CLOCKS_PER_SEC));

	// if (server_init(game, game->server, "0.0.0.0", 7777) < 0) {
	//	_ERROR("Initializing TCP server for game context failed.\n");
	//	ret = -1;
	//	goto out;
	//}

	//   game->server->game = game; // TODO: Fix this obvious segfault

	// if (server_start(game->server) < 0) {
	//	_ERROR("Starting server for game context failed.\n");
	//	ret = -1;
	//	goto out;
	//}

	// ptGameInitializeUpdateLoop(game);

	// printf("\nStarted successfully on %s:%d\n", game->server->listen_address,
	// game->server->port);

	// printf(" * %s (%dx%d)\n", world.world_name, world.max_tiles_x,
	// world.max_tiles_y);

	// printf(" * Expert: %s, Crimson: %s\n", game->world->expert_mode ? "Yes" :
	// "No", 	   game->world->flags.crimson ? "Yes" : "No");

	// printf("\n");

	// if (options_console == true && console_init(game->console, game) < 0) {
	//	_ERROR("Initializing console failed.\n");
	//	ret = -1;
	//	goto out;
	//}

	// ptGameStartEventLoop(game);

	// uv_walk(&game->eventLoop, __close_handle, NULL);
	// uv_run(&game->eventLoop, UV_RUN_DEFAULT);

	// if (uv_loop_close(&game->eventLoop) == UV_EBUSY) {
	//	printf("%s: warning: event loop has not signalled a close.",
	//__FUNCTION__);
	//}

	_CrtDumpMemoryLeaks();

	return ret;
}

#ifdef __cplusplus
}
#endif
