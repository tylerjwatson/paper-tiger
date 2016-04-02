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

#include <uv.h>
#include <stdint.h>

#include "talloc/talloc.h"

#define PACKET_HEADER_SIZE 3
#define PACKET_PAYLOAD_SIZE 0xFFFF

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup packet Packet subsystem
 * @ingroup paper-tiger 
 * 
 * The packet subsystem provides a mechanism to read and write encoded Terraria messages to
 * Terraria clients.
 * 
 * All Terraria messages have a header consisting of 3 bytes; the packet length as a 16-bit 
 * unsigned integer, and a single-byte message type specifier.
 * 
 * Terraria messages are all encoded into one packet structure (`struct packet`) which
 * contains the header fields and a generic pointer (called `data`) which points to the structure
 * containing the body of the message, decoded by the packet system by a call to the registered **Read**
 * handler for that packet type.
 * 
 * Each packet has its own implementation in `src/packets` and are handled in an object-oriented
 * manner.  Packet implementations must implement any number of three functions in the packet handler
 * table in `src/packets.c`:
 * 
 * Function | Prototype                                            | Description
 * ---------|------------------------------------------------------|----------
 *  Read    | `int (*packet_read_cb)(struct packet *packet, uv_buf_t *buf)` | Packet read functions are responsible for filling the body of the `struct packet` out once the header information has been parsed.  Memory for the packet body must be `talloc`ated underneath the `struct packet` so it may be freed when the packet is released.  At the return of this handler function, the `data` member of `struct packet` must point to a valid structure containing the contents of the packet body.
 *  Write   | `int (*packet_write_cb)(TALLOC_CTX *context, const struct packet *packet, const struct player *player, uv_buf_t *buffer)` | Packet write functions are responsible for serializing the `struct packet` instance into a buffer for sending to a client.  This function is responsible for allocating enough room in the uv_buf_t structure to accomodate the entire message which must be `talloc`ated underneath the context pointed to by `context`.
 *  Handle  | `int (*packet_handle_cb)(struct player *player, struct packet *packet)` | Packet handle functions are called in order for the server to do something with a message that has been received from the client.  Implement this function to do something with a packet once it has been received.  **Note:** Do not directly free the structure in the handle functions, as they are destroyted automatically at the return of this function.
 * 
 * Packet implementations may provide `new` constructor functions to aid creating of packets in order
 * for them to be sent to clients via `server_send_packet`.
 * 
 * @{
 */

struct player;
struct game;

/**
 * Describes a Terraria message.  Contains the packet header information and a pointer to
 * the concrete object which contains the body of the Terraria message.
 */
struct packet {
	/**
	 * The packet length including the 3-byte message header, in bytes.
	 */
	uint16_t len;
	
	/**
	 * The Terraria message type.
	 */
	uint8_t type;

	/**
	 * A pointer to the packet payload object that was deserialized using the `packet_read_cb` function
	 * inside the packet handler table.
	 */
	void *data;
};

/**
 * @brief Function to call in order for the server to do something with a message that has been received from the client.
 * 
 * Implement this function to do something with a packet once it has been received.  **Note:** Do not directly free 
 * the structure in the handle functions, as they are destroyted automatically at the return of this function.
 * 
 * @param[in] context
 * The talloc context object in which any buffer memory must be allocated underneath, so it may be freed when the packet is
 * no longer in use.
 * 
 * @param[in] packet
 * An instance of the packet of the same type to have its contents serialized
 * 
 * @param[in] player
 * A player who will receive the encoded message once it has been filled into the buffer for sending
 * 
 * @param[in] buffer
 * A pointer to an unallocated uv_buf_t structure.  **The write handler is responsible for allocating the buffer structure
 * and its buffers with enough room to contain the encoded packet**.  The buffer will be freed when the talloc context pointed
 * to by @a context is freed.
 * 
 * @returns
 * `0` if the encoding was successful, `< 0` otherwise.
 */
typedef int (*packet_write_cb)(const struct game *game, const struct packet *packet, uv_buf_t buffer);

/**
 * @brief Function to call to translate a packet from a network buffer to a fully-qualified `struct packet`.
 * 
 * Packet read functions are responsible for filling the body of the `struct packet` out once the header 
 * information has been parsed.  Memory for the packet body must be `talloc`ated underneath the 
 * `struct packet` so it may be freed when the packet is released.  At the return of this handler function, 
 * the `data` member of `struct packet` must point to a valid structure containing the contents of the packet body.
 * 
 * @param[in] packet
 * A pointer to a partially-allocated packet structure.  The `len` and `type` members of this function are pre-filled
 * 
 * @param[in] buffer
 * A pointer to an allocated network buffer in which to deserialize the Terraria message from.
 * 
 * @returns
 * `0` if the decoding was successful, `< 0` otherwise.
 * 
 * @remarks
 * This function is responsible for setting the `data` member of the packet structure pointed to by @a packet.
 * All memory for the packets body must be `talloc`ated underneath the @a packet context so it may be freed 
 * when the packet is no longer required.
 * 
 * At the return of this function, the `data` member of the packet must point to a valid packet body, or 
 * `NULL` if the packet does not have a body.
 */
typedef int (*packet_read_cb)(struct packet *packet, const uv_buf_t *buffer);

/**
 * @brief Function to be called when a message is to be handled by the server.
 * 
 * This function gets called by the packet subsystem when it has a message for the server to process.
 * 
 * @param[in] player
 * A pointer to the player who sent the message
 * 
 * @param[in] packet
 * A pointer to the message received from the player
 * 
 * @returns
 * `0` if the packet handling succeeded, `< 0` otherwise, or if the handler would like the server to 
 * fatally exit upon a condition.
 */
typedef int (*packet_handle_cb)(struct player *player, struct packet *packet);

/**
 * Describes a packet handler in the packet handler table.  Contains function pointers to the implementation
 * functions to read, write and handle Terraria messages.
 * 
 * These are statically set in the packet handlers table.
 */
struct packet_handler {
	uint8_t type;
	packet_read_cb read_func;
	packet_handle_cb handle_func;
	packet_write_cb write_func;
};

/**
 * @brief Retrieves a pointer to the registered packet handler for the message type specified.
 * 
 * @param[in] type
 * The type of message to retrieve the handler structure for
 * 
 * @returns
 * A pointer to the packet handler if one owas found in the packet handler table if one was
 * found by the message type, or NULL if one was not found or there was an error.
 */
struct packet_handler *packet_handler_for_type(uint8_t type);

void packet_write_header(uint8_t type, uint16_t len, uv_buf_t *buf, int *pos);

int packet_read_header(const uv_buf_t *buf, uint8_t *out_type, uint16_t *out_len);

int packet_new(TALLOC_CTX *ctx, struct player *player, const uv_buf_t *buf, struct packet **out_packet);

#ifdef __cplusplus
}
#endif
