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

#ifndef _HAVE_GAME_H
#define _HAVE_GAME_H

#include <errno.h>
#include <stdbool.h>
//#include <pthread.h>
#include <uv.h>

#include "talloc/talloc.h"
#include "bitmap.h"
#include "server.h"
#include "player.h"

#define GAME_MAX_PLAYERS 255

struct game_context {
	bool is_exited;
	double ms_per_frame;

	struct world *world;
    struct server *server;
    struct player *players[256];
	
	word_t *player_slots;
	
    uv_loop_t *event_loop;
	/*pthread_t game_thread;
	pthread_mutex_t game_mutex;*/
};

int game_find_next_slot(struct game_context *context);

int game_start_event_loop(struct game_context *context);

int game_start_thread(struct game_context *context);

int game_run(struct game_context *context);

int game_new(TALLOC_CTX *context, struct game_context **out_context);

#endif /* _HAVE_GAME_H */
