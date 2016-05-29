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

#include "player.h"
#include "hook.h"
#include "packet.h"
#include "server.h"
#include "bitmap.h"
#include "dataloader.h"
#include "util.h"
#include "config.h"
#include "world.h"
#include "colour.h"

#include "packets/tile_section.h"
#include "packets/section_tile_frame.h"
#include "packets/chat_message.h"
#include "packets/status.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define FRAMES_PER_SEC 60

#define TIME_NOON 27000
#define TIME_MIDNIGHT 16200
#define TIME_DUSK 0

static int game_send_world(const struct game *game, const struct player *player);

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

static int game_on_player_leave(const struct game *game, const struct player *player)
{
	game_send_message(game, player, colour_black, "%s v%d.%d.",
		PRODUCT_NAME, VERSION_MAJOR, VERSION_MINOR);

	console_vsprintf(game->console,
		"\033[31;1m* %s has left.\033[0m\n",
		player->name);

	return 0;
}

static int game_on_player_join(const struct game *game, const struct player *player)
{
	game_send_message(game, player, colour_black, "%s v%d.%d.",
		PRODUCT_NAME, VERSION_MAJOR, VERSION_MINOR);

	console_vsprintf(game->console, 
		"\033[32;1m* %s has joined.  Slot=%d IP=%s\033[0m\n", 
		player->name,
		player->id,
		player->remote_addr);

	game_send_world(game, player);

	return 0;
}

static int game_init(struct game *game)
{
	int ret = -1;

	hook_player_join_register(game->hooks, game_on_player_join);
	hook_player_leave_register(game->hooks, game_on_player_leave);

	ret = 0;

	return ret;
}

int game_new(TALLOC_CTX *context, struct game **out_context)
{
	int ret = -1;
	struct game *game = NULL;
	TALLOC_CTX *tempContext;

	if ((tempContext = talloc_new(NULL)) == NULL) {
		return -ENOMEM;
	}

	game = talloc_zero(tempContext, struct game);

	/*
	 * Init game stuff here
	 */
	game->ms_per_frame = 1000. / FRAMES_PER_SEC;

	if ((game->event_loop = talloc_zero(game, uv_loop_t)) == NULL) {
		_ERROR("%s: Could not allocate a game event loop.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}
	
	uv_loop_init(game->event_loop);
	game->player_slots = talloc_zero_array(game, word_t, GAME_MAX_PLAYERS / sizeof(word_t));
	talloc_set_destructor(game, __game_destructor);
	
	if (dataloader_load_tile_flags(game) < 0) {
		_ERROR("%s: loading tile flags failed.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (hook_context_new(game, game, &game->hooks) < 0) {
		_ERROR("%s: initializing hook subsystem failed.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	game_init(game);

	*out_context = talloc_steal(context, game);

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

	if (chat_message_new((TALLOC_CTX *)player, player, colour, msg, &chat_packet) < 0) {
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
			out_ids[count++] = i;
		}
	}

	return count;
}

static int game_send_world(const struct game *game, const struct player *player)
{
	int ret = -1, num_packets;
	struct packet *status, *tile_section, *section_frame;
	TALLOC_CTX *temp_context;
	
	struct rect tile, section;

	num_packets = game->world->max_sections_x * game->world->max_sections_y * 2;

	temp_context = talloc_new(game);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for world send.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	if (status_new(temp_context, player, num_packets, "Downloading the rest of the map", &status) < 0) {
		_ERROR("%s: out of memory allocating status packet for world sending.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	server_send_packet(game->server, player, status);

	for (uint8_t y = 0; y < game->world->max_sections_y; y++)
	for (uint8_t x = 0; x < game->world->max_sections_x; x++) {
		section.x = x;
		section.y = y;
		section.w = x + 1;
		section.h = y + 1;

		tile.x = x * WORLD_SECTION_WIDTH;
		tile.y = y * WORLD_SECTION_HEIGHT;
		tile.w = WORLD_SECTION_WIDTH;
		tile.h = WORLD_SECTION_HEIGHT;

		if (tile_section_new(temp_context, player, tile, &tile_section) < 0) {
			_ERROR("%s: out of memory allocating status packet for world sending.\n", __FUNCTION__);
			ret = -ENOMEM;
			goto out;
		}

		if (section_tile_frame_new(temp_context, player, section, &section_frame) < 0) {
			_ERROR("%s: out of memory allocating status packet for world sending.\n", __FUNCTION__);
			ret = -ENOMEM;
			goto out;
		}

		server_send_packet(game->server, player, tile_section);
		server_send_packet(game->server, player, section_frame);

		talloc_free(tile_section);
		talloc_free(section_frame);
	}

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}
