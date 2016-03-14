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

#ifndef _HAVE_PACKET_H
#define _HAVE_PACKET_H

#include <uv.h>
#include <stdint.h>

#include "player.h"

#define PACKET_HEADER_SIZE 3

#ifdef __cplusplus
extern "C" {
#endif

struct packet {
	uint16_t len;
	uint8_t type;

	struct player *player;

	void *data;
};

typedef int (*packet_write_cb)(const struct packet *packet, const uv_buf_t *buffer);
typedef int (*packet_read_cb)(struct packet *packet, const uv_buf_t *buffer);
typedef int (*packet_handle_cb)(const struct player *player, struct packet *packet);

struct packet_handler {
	uint8_t type;
	packet_read_cb read_func;
	packet_handle_cb handle_func;
	packet_write_cb write_func;
};

struct packet_handler *packet_handler_for_type(uint8_t type);

int packet_read_header(const uv_buf_t *buf, uint8_t *out_type, uint16_t *out_len);

int packet_new(TALLOC_CTX *ctx, struct player *player, const uv_buf_t *buf, struct packet **out_packet);

int packet_send(uint8_t packet_type);

#ifdef __cplusplus
}
#endif

#endif /* _HAVE_PACKET_H */
