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
 
 * paper-tiger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with paper-tiger.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "game.h"

#include <stdarg.h>
#include <time.h>
#include <uv.h>

#include "server.h"
#include "bitmap.h"
#include "dataloader.h"
#include "util.h"
#include "colour.h"

#include "packets/chat_message.h"

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

static void __game_update(uv_timer_t *timer)
{
	struct game *game = (struct game *)timer->data;
 
	//TODO: Update the game shit.
}

static int __game_destructor(struct game *context)
{   
}

int game_find_next_slot(struct game *context)
{
	for (unsigned i = 0; i < GAME_MAX_PLAYERS; i++) {
		if (bitmap_get(context->player_slots, i) == false) {
			bitmap_set(context->player_slots, i);
			return i;
		}
	}

	return -1;
}

int game_start_event_loop(struct game *context)
{
	return uv_run(context->event_loop, UV_RUN_DEFAULT);
}

int game_new(TALLOC_CTX *context, struct game **out_context)
{
	int ret = -1;
	struct game *gameContext = NULL;
	TALLOC_CTX *tempContext;

	if ((tempContext = talloc_new(NULL)) == NULL) {
		return -ENOMEM;
	}

	gameContext = talloc_zero(tempContext, struct game);

	/*
	 * Init game stuff here
	 */
	gameContext->ms_per_frame = 1000. / FRAMES_PER_SEC;

	if ((gameContext->event_loop = talloc_zero(gameContext, uv_loop_t)) == NULL) {
		_ERROR("%s: Could not allocate a game event loop.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}
	
	uv_loop_init(gameContext->event_loop);
	gameContext->player_slots = talloc_zero_array(gameContext, word_t, GAME_MAX_PLAYERS / sizeof(word_t));
	talloc_set_destructor(gameContext, __game_destructor);
	
	if (dataloader_load_tile_flags(gameContext) < 0) {
		_ERROR("%s: loading tile flags failed.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}
	
	*out_context = talloc_steal(context, gameContext);

	ret = 0;
out:
	talloc_free(tempContext);
	return ret;
}

int game_update_loop_init(struct game *game)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	uv_timer_t *update_timer;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: Allocating a temp context for game update timer failed.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	update_timer = talloc(temp_context, uv_timer_t);
	if (update_timer == NULL) {
		_ERROR("%s: Allocating an update timer failed.\n", __FUNCTION__);
		goto out;
	}
	
	uv_timer_init(game->event_loop, update_timer);
	update_timer->data = game;
	uv_timer_start(update_timer, __game_update, (uint64_t)game->ms_per_frame, (uint64_t)game->ms_per_frame);

	game->update_handle = talloc_steal(game, update_timer);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int game_send_message(const struct game *game, const struct player *player, const struct colour colour, 
					  const char *fmt, ...)
{
	struct packet *chat_packet;
	
	char msg[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(msg, 1024, fmt, args);
	va_end(args);

	if (chat_message_new(player, player, colour, msg, &chat_packet) < 0) {
		_ERROR("%s: out of memory allocating chat packet.\n", __FUNCTION__);
		return -ENOMEM;
	}

	((struct chat_message *)chat_packet->data)->id = 0xFF; //nameless broadcast
	
	server_send_packet(game->server, player, chat_packet);

	return -1;
}

int game_online_players(const struct game *game, uint8_t *out_ids)
{
	int count = 0;

	for (uint8_t i = 0; i < GAME_MAX_PLAYERS; i++) {
		if (bitmap_get(game->player_slots, i) == true) {
			out_ids[count++] == i;
		}
	}

	return count;
}