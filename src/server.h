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

#pragma once

#include <uv.h>

#include "talloc/talloc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct packet;
struct player;

/**
* @defgroup server Server subsystem
* @ingroup paper-tiger
* 
* The server subsystem lives inside the `game` context, and is responsible for managing
* and talking to connected clients via the _Terraria_ protocol.  The `server` system has
* routines for decoding and encoding _Terraria_ messages, as well as callbacks for managing
* the TCP connections to all its clients.
* 
* @{
*/

/**
 * Describes a server context.
 */
struct server {
	/**
	 * The interface address on which to listen on, use `"0.0.0.0"` for any, or an
	 * address to restrict listening on a specific interface.
	 */
    char *listen_address;
	
	/**
	 * TCP port on which to listen on.
	 */
    uint32_t port;
	
	/**
	 * Backpointer to the game context in which this server is running.
	 */
    struct game *game;
    
	/**
	 * libuv TCP handle which is the TCP listener presenting network I/O on the game's event
	 * loop.
	 */
    uv_tcp_t *tcp_handle;
};

/**
 * @brief Allocates a new server context, with the specified listen address and port.
 * 
 * Allocates a new server object underneath the specified talloc context, with the
 * specified @a listen_address and @a port.
 * 
 * @param[in] context
 * The talloc context in which to allocate memory for this server context underneath.
 * 
 * @param[in] listen_address
 * The IPv4 interface listen address to listen for TCP connections on.
 * 
 * @param[in] port
 * Specifies the TCP port on which to listen on
 * 
 * @param[in] game
 * A pointer to the game context in which to allocate a server object under
 * 
 * @param[out] out_server
 * A pointer to the address in which the allocated server structure may be stored.
 * 
 * @returns
 * `0` if the server context was allocated successfully, `< 0` if an error occurred.
 */
int server_new(TALLOC_CTX *context, const char *listen_address, const uint16_t port,
               struct game *game, struct server **out_server);

/**
 * @brief starts the server
 * 
 * Starts the server pointed to by @a server, by opening a server socket on the specified
 * TCP address and port, and listening for clients on the game context's event loop.
 * 
 * @param[in] server
 * A pointer to an allocated server object in which to start listening for clients.
 *
 * @returns
 * `0` if the server start operation was successful, `< 0` otherwise.
 * 
 * @remarks
 * This function will return `-1` if it cannot listen for clients, for reasons such as socket
 * bind failures, port open failures, AVC denies, etc.
 */
int server_start(struct server *server);

/**
 * @brief Sends a packet to the specified player.
 * 
 * Sends a packet pointed to by @a packet, to the client pointed to by @a player.
 * 
 * @param[in] player
 * A pointer to an allocated and joined player object whom to send the packet
 * 
 * @param[in] packet
 * A pointer to an allocated packet to send to the player.
 * 
 * @returns
 * `0` if the packet was queued for sending successfully, `< 0` otherwise.
 * 
 * @remarks
 * The successful return from this function does **not** indicate that the packet was
 * sent successfully and arrived at its destination.  It merely guarantees that the
 * packet was queued for sending to the client at the return of this function. 
 * The packet buffers rely on function callbacks in the packet handler table to be
 * able to send the data to the client.  The packet's data stream is copied from the
 * packet into the send queue, thus the caller of this function still owns the memory
 * pointed to by @a packet and is responsible for freeing it.
 */
int server_send_packet(const struct server *server, const struct player *player,
	const struct packet *packet);


int server_broadcast_packet(const struct server *server, const struct packet *packet);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */
