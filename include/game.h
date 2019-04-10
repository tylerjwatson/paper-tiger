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

#pragma once

#include <errno.h>
#include <stdbool.h>
#include <uv.h>

#include "bitmap.h"
#include "colour.h"

#include "talloc/talloc.h"

#define GAME_MAX_PLAYERS 255
//#define GAME_PROTOCOL_VERSION 156 << this is for 1.3.0
#define GAME_PROTOCOL_VERSION 169 // this is for 1.3.1

#ifdef _WIN32
#include <WinSock2.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup game Game system
 * @ingroup paper-tiger
 *
 * The game system is the root of all server mechanics and serves as the game environment.
 * In the `talloc` heirarchy, all memory is allocated underneath the game context, and its
 * free should close and release all memory and subsystems to do with the game context.  In
 * other words, freeing the game context should completely stop and release the entire game.
 *
 * As paper tiger is object-oriented, the game environment is not set in globals
 * or statics. Any data related to the the game or the game's environment must be
 * allocated underneath the game context in the talloc heirarchy, and stored as
 * one of the members in the game structure, or any substructure underneath the
 * game context.
 *
 * The game context contains references to the **world** systems, the **player**
 * systems, and the **server** systems so that none of the systems exist in a fixed
 * address space and may be fully cleaned up on a call to `talloc_free(game)`.
 *
 * @{
 */

/**
 * Describes a game context.  Contains all references and information that a game needs
 * in order to run.
 */
typedef struct ptGame
{
    /**
	* Array of players.
	*
	* @remarks
	* Due to the limitations of the official Terraria server, the maximum number of
	* players supported is 255.
	*/
    //struct player players[GAME_MAX_PLAYERS];

    /**
        * Bitmap of connected players, decides which slot ID connecting clients receive
        */
    word_t player_slots[GAME_MAX_PLAYERS / sizeof(word_t)];

    /**
	* Main libuv event loop for the game.
	*/
    uv_loop_t *eventLoop;

    /**
	* libuv timer which fires every @a msPerFrame ms.  Serves as the game update loop.
	*/
    uv_timer_t *update_handle;

    /**
	* Array of tile frame important data.
	*/
    bool tileFrameImportant[419];


    /** libuv handle for TCP server. */
    uv_tcp_t tcpHandle;

    /** libub handle for working with the console */
    uv_work_t consoleThread;
} ptGame;

/**
 * Initializes a game.
 *
 * @param {ptGame} game
 * A pointer to a ptGame instance to initialize
 */
int ptGameInitialize(ptGame* game, uv_loop_t *loop);


/**
 * @brief finds the next free slot from the game's player slot bitmap
 *
 * Finds the next free slot in the game context's player slot bitmap and returns it
 * to the caller.  Exchanges the slot to `1 (used)` before returning it, so subsequent
 * calls to this function will not return the same ID.
 *
 * @param[in]		context		A pointer to the game context in which to find the next
 * 								free player slot for
 *
 * @returns
 * The ID of the next free player slot (`>= 0`) if a player slot exists, or -1 if there
 * are no more slots available or there was an error.
 */
int
ptGameFindSlot(ptGame *context);

/**
 * @brief starts the game's event loop
 *
 * Starts the game's event loop.  This function will block until the loop is closed
 * with calls to @a uv_close, or the loop somehow otherwise terminates.
 *
 * @param[in]		context		A pointer to the game context in which to start the
 * 								event loop.
 *
 * @returns
 * This function currently always returns `0`.
 */
int
ptGameStartEventLoop(ptGame *context);

/**
 * @brief Allocates a new game context.
 *
 * Allocates a new game context under a parent talloc context pointed to by @a context.
 */
int
ptGameAlloc(TALLOC_CTX *context, ptGame **out_context);

/**
 * @brief Initializes the game update timer which fires at every tickrate interval.
 *
 * @param[in]		game		A pointer to the game context in which to start the
 * 								timer
 *
 * @returns
 * `0` if the update loop was initialized successfully, `< 0` otherwise.
 *
 * @remarks
 * The game update timer is a libuv timer, which is smart enough to adjust itself
 * according to the time it takes for the game update procedure to finish.
 */
int
ptGameInitializeUpdateLoop(ptGame *game);
//
//int
//ptGameSendMessageToPlayer(const ptGame *game, const struct player *player,
//                          const struct colour colour, const char *fmt, ...);

int
ptGameOnlinePlayerSlots(const ptGame *game, uint8_t *out_ids);
//
//int
//ptGameSyncPlayers(const ptGame *game, const struct player *new_player);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
