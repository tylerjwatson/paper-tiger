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

#include "bitmap.h"
#include "config.h"
#include "log.h"

#ifdef _WIN32
#else
#include <unistd.h>
#endif

#define FRAMES_PER_SEC 60

#define TIME_NOON 27000
#define TIME_MIDNIGHT 16200
#define TIME_DUSK 0

static int tileFrameImportant[53] = {
	0x38, 0xFC, 0x3F, 0xBD, 0x1E, 0x04, 0x84, 0x20, 0x80, 0xE7, 0xFE,
	0xFF, 0xFF, 0x47, 0x06, 0x60, 0xF3, 0xEF, 0x21, 0x00, 0x20, 0x78,
	0x04, 0x0F, 0x00, 0x82, 0x96, 0x1F, 0x98, 0xFA, 0xFF, 0x40, 0x00,
	0xE0, 0xF8, 0xEF, 0xFF, 0xFF, 0x7F, 0xF4, 0x19, 0xC0, 0x0E, 0x20,
	0xDC, 0x1F, 0xF0, 0x17, 0xFC, 0x0F, 0x60, 0x7C, 0x00};

static void
ptGameOnUpdate(uv_timer_t *timer)
{
	ptGame *game = (ptGame *)timer->data;

	(void)game; // unused
				// TODO: Update the game shit.
}

static int
ptGameOnDestroy(ptGame *context)
{
	return 0;
}

int
ptGameFindSlot(ptGame *context)
{
	for (unsigned i = 0; i < GAME_MAX_PLAYERS; i++) {
		if (bitmap_get(context->player_slots, i) == false) {
			bitmap_set(context->player_slots, i);
			return i;
		}
	}

	return -1;
}

int
ptGameStartEventLoop(ptGame *context)
{
	return uv_run(context->eventLoop, UV_RUN_DEFAULT);
}

static int
ptGameInitializeServer(ptGame *game)
{
	int r = -1;
	const struct sockaddr_in addr;

	if ((r = uv_tcp_init(game->eventLoop, &game->tcpHandle)) < 0) {
		log_fatal("Cannot initialize TCP socket: %d", r);
		goto out;
	}

	uv_tcp_nodelay(&game->tcpHandle, true);
	uv_ip4_addr("127.0.0.1", //TODO: cvar
				7777 /* TODO: cvar */, (struct sockaddr_in *)&addr);

	if ((r = uv_tcp_bind(&game->tcpHandle, (const struct sockaddr *)&addr,
						 0U)) < 0) {
		// log_fatal("Cannot bind TCP socket to %s:%d: %d",
			//    game->properties.listenAddr, game->properties.listenPort, r);
		goto out;
	}

	/* Push pointer to game handle into libuv for backreferencing in handlers */
	game->tcpHandle.data = game;

	r = 0;

	/* fall through */
out:
	return r;
}

/**
 * Function called when a new connection comes in.
 */
static void
ptGameServerOnNewConnection(uv_stream_t *handle, int status)
{
	if (status < 0) {
		log_fatal("Cannot accept new connection.");
		return;
	}
}

static int
ptGameServerListen(ptGame *game)
{
	int ret = -1;

	if ((ret = uv_listen((uv_stream_t *)&game->tcpHandle, 128,
						 ptGameServerOnNewConnection)) < 0) {
		log_fatal("Cannot listen on server socket: %d", ret);

		return ret;
	}

	ret = 0;

	return ret;
	// log_info("TCP server listening on %s:%d", game->properties.listenAddr,
	// 		 game->properties.listenPort);
}

int
ptGameInitialize(ptGame *game, uv_loop_t *loop)
{
	int ret = -1;
	/*
		hook_player_join_register(game->hooks, ptGameOnPlayerJoin);
		hook_player_leave_register(game->hooks, ptGameOnPlayerLeave);
        */

	memcpy(game->tileFrameImportant, tileFrameImportant,
		   sizeof(tileFrameImportant));
	game->eventLoop = loop;

	if ((ret = ptGameInitializeServer(game)) < 0) {
		log_fatal("Initializing server failed: %d", ret);
		return ret;
	}

	if ((ret = ptGameServerListen(game) < 0)) {
		log_fatal("Listen socket failed: %d", ret);
		return ret;
	}

    ret = 0;

	return ret;
}
//
// int
// ptGameInitializeUpdateLoop(ptGame *game)
//{
//	int ret = -1;
//	TALLOC_CTX *temp_context;
//	uv_timer_t *update_timer;
//
//	temp_context = talloc_new(NULL);
//	if (temp_context == NULL) {
//		log_fatal("%s: Allocating a temp context for game update timer failed.\n",
//			   __FUNCTION__);
//		ret = -ENOMEM;
//		goto out;
//	}
//
//	update_timer = talloc(temp_context, uv_timer_t);
//	if (update_timer == NULL) {
//		log_fatal("%s: Allocating an update timer failed.\n", __FUNCTION__);
//		goto out;
//	}
//
//	uv_timer_init(&game->eventLoop, update_timer);
//	update_timer->data = game;
//	uv_timer_start(update_timer, ptGameOnUpdate, (uint64_t)game->msPerFrame,
//				   (uint64_t)game->msPerFrame);
//
//	game->update_handle = talloc_steal(game, update_timer);
//
//	ret = 0;
// out:
//	talloc_free(temp_context);
//
//	return ret;
//}

int
ptGameOnlinePlayerSlots(const ptGame *game, uint8_t *out_ids)
{
	int count = 0;

	for (uint8_t i = 0; i < GAME_MAX_PLAYERS; i++) {
		if (!bitmap_get(game->player_slots, i)) {
			out_ids[count++] = i;
		}
	}

	return count;
}
//
// int
// ptGameSyncPlayers(const ptGame *game, const struct player *new_player)
//{
//	struct packet *player_info, *hp, *mp, *spawn;
//	int ret = -1;
//
//	if (player_info_new((void *)game, new_player, &player_info) < 0) {
//		log_fatal("%s: player_info packet failed.\n", __FUNCTION__);
//		ret = -1;
//		goto out;
//	}
//
//	if (player_hp_new((void *)game, new_player, new_player->life,
//					  new_player->life_max, &hp) < 0) {
//		log_fatal("%s: player_hp packet failed.\n", __FUNCTION__);
//		ret = -1;
//		goto player_hp_out;
//	}
//
//	if (player_mana_new((void *)game, new_player, new_player->mana,
//						new_player->mana_max, &mp) < 0) {
//		log_fatal("%s: player_mana packet failed.\n", __FUNCTION__);
//		ret = -1;
//		goto player_mana_out;
//	}
//
//	server_broadcast_packet(game->server, player_info, new_player->id);
//	server_broadcast_packet(game->server, hp, new_player->id);
//	server_broadcast_packet(game->server, mp, new_player->id);
//
//	ret = 0;
//
//	/*
//	 * Fall through
//	 */
//	talloc_free(mp);
//
// player_mana_out:
//	talloc_free(hp);
//
// player_hp_out:
//	talloc_free(player_info);
//
// out:
//	return ret;
//}
//
// int
// ptGameSendWorld(const ptGame *game, const struct player *player)
//{
//	int ret = -1, num_packets;
//	struct packet *status, *tile_section, *section_frame;
//	TALLOC_CTX *temp_context;
//
//	struct vector_2d section_coords;
//
//	num_packets = game->world->max_sections_x * game->world->max_sections_y * 2;
//
//	temp_context = talloc_new(game);
//	if (temp_context == NULL) {
//		log_fatal("%s: out of memory allocating temp context for world send.\n",
//			   __FUNCTION__);
//		ret = -ENOMEM;
//		goto out;
//	}
//
//	if (status_new(temp_context, player, num_packets,
//				   "Downloading the rest of the map", &status) < 0) {
//		log_fatal(
//			"%s: out of memory allocating status packet for world sending.\n",
//			__FUNCTION__);
//		ret = -ENOMEM;
//		goto out;
//	}
//
//	server_send_packet(game->server, player, status);
//
//	for (unsigned section = 0; section < game->world->max_sections; section++) {
//		section_coords =
//			world_section_num_to_coords(player->game->world, section);
//
//		if (tile_section_new(temp_context, player, section, &tile_section) <
//			0) {
//			log_fatal("%s: out of memory allocating status packet for world "
//				   "sending.\n",
//				   __FUNCTION__);
//			ret = -ENOMEM;
//			goto out;
//		}
//
//		if (section_tile_frame_new(temp_context, player, section_coords,
//								   &section_frame) < 0) {
//			log_fatal("%s: out of memory allocating status packet for world "
//				   "sending.\n",
//				   __FUNCTION__);
//			ret = -ENOMEM;
//			goto out;
//		}
//
//		server_send_packet(game->server, player, tile_section);
//		server_send_packet(game->server, player, section_frame);
//
//		talloc_free(tile_section);
//		talloc_free(section_frame);
//	}
//
//	ret = 0;
// out:
//	talloc_free(temp_context);
//
//	return ret;
//}
