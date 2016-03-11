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

#include "packet.h"

static int handle_connect_request(const struct player *player, const struct packet_buffer *buf)
{
	return 0;
}

static struct packet_handler packet_handlers[] = {
	{ .type = PACKET_TYPE_CONNECT_REQUEST, .handler = handle_connect_request }
};

int packet_read_header(const uv_buf_t *buf, uint8_t *out_type, uint16_t *out_len)
{
	if (buf->len <= 3) {
		return -1;
	}

	return 0;
}

