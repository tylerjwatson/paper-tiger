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
 
 * upgraded-guacamole is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with upgraded-guacamole.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <time.h>
#include <uv.h>

#include "game.h"
#include "util.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define FRAMES_PER_SEC 60

#define TIME_NOON 27000
#define TIME_MIDNIGHT 16200
#define TIME_DUSK 0

static inline void __sleep(double msec)
{
#if _WIN32
	Sleep((DWORD)msec);
#else
	usleep((useconds_t)(msec * 1000));
#endif
	
}

static int __game_update(struct game_context *context)
{
	return 0;
}

static void *__game_thread(void *data)
{
	struct game_context *context = (struct game_context *)data;

	game_run(context);
    
    return NULL;
}

static void __game_destructor(struct game_context *context)
{
	context->is_exited = true;
	//pthread_join(context->game_thread, NULL);
 //   pthread_mutex_destroy(&context->game_mutex);
    
    uv_loop_close(context->event_loop);
}

int game_find_next_slot(struct game_context *context)
{
	for (unsigned i = 0; i < GAME_MAX_PLAYERS; i++) {
		if (bitmap_get(context->player_slots, i) == false) {
			bitmap_set(context->player_slots, i);
			return i;
		}
	}
	
	return -1;
}

int game_start_event_loop(struct game_context *context)
{
    return uv_run(context->event_loop, UV_RUN_DEFAULT);
}

int game_start_thread(struct game_context *context)
{
	//if (pthread_create(&context->game_thread, NULL, __game_thread, (void *)context) < 0) {
	//	_ERROR("%s: Could not create game thread.\n", __FUNCTION__);
	//	return -1;
	//}
    
    return 0;
}

int game_run(struct game_context *context)
{
	double msec;
	int ret;
	clock_t start, diff;

	do {
		start = clock();

		if ((ret = __game_update(context)) < 0) {
			_ERROR("%s: game has existed with code %d", __FUNCTION__, ret);
			return ret;
		}

		diff = clock() - start;
		msec = diff * 1000. / CLOCKS_PER_SEC;

		printf("%s: frame took %f ms\n", __FUNCTION__, msec);
		if (context->ms_per_frame - msec > 0) {
			__sleep(context->ms_per_frame - msec);
		}
	} while (context->is_exited == false);

	return 0;
}

int game_new(TALLOC_CTX *context, struct game_context **out_context)
{
	int ret = -1;
	struct game_context *gameContext = NULL;
	TALLOC_CTX *tempContext;

	if ((tempContext = talloc_new(NULL)) == NULL) {
		return -ENOMEM;
	}

	gameContext = talloc_zero(tempContext, struct game_context);
    
	/*
	 * Init game stuff here
	 */

	gameContext->ms_per_frame = 1000. / FRAMES_PER_SEC;

    if ((gameContext->event_loop = talloc_zero(gameContext, uv_loop_t)) == NULL) {
        _ERROR("%s: Could not allocate a game event loop.\n", __FUNCTION__);
        ret = -1;
        goto out;
    }
    

	//if (pthread_mutex_init(&gameContext->game_mutex, NULL) < 0) {
	//	_ERROR("%s: Could not initialize game thread mutex\n", __FUNCTION__);
	//	ret = -1;
	//	goto out;
	//}
    
    uv_loop_init(gameContext->event_loop);
	
	gameContext->player_slots = talloc_array(gameContext, word_t, GAME_MAX_PLAYERS / sizeof(word_t));
	
	talloc_set_destructor(gameContext, __game_destructor);

	*out_context = talloc_steal(context, gameContext);

	ret = 0;
out:
	talloc_free(tempContext);
	return ret;
}
