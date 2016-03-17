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

#include <stdint.h>

#include "packet.h"
#include "util.h"

#include "packets/connect_request.h"
#include "packets/continue_connecting.h"

static struct packet_handler packet_handlers[] = {
	{ .type = PACKET_TYPE_CONNECT_REQUEST, .read_func = connect_request_read, .handle_func = connect_request_handle },
	{ .type = PACKET_TYPE_CONTINUE_CONNECTING, .read_func = NULL, .handle_func = NULL, .write_func = continue_connecting_write },
	{ 0x00, NULL, NULL }
};

struct packet_handler *packet_handler_for_type(uint8_t type)
{
	struct packet_handler *handler;

	for (handler = packet_handlers; handler->type != 0x00; handler++) {
		if (handler->type == type) {
			return handler;
		}
	}

	return NULL;
}

int packet_read_header(const uv_buf_t *buf, uint8_t *out_type, uint16_t *out_len)
{
	if (buf->len < 3) {
		return -1;
	}

	*out_len = *(uint16_t *)buf->base;
	*out_type = buf->base[2];

	return 0;
}

int packet_new(TALLOC_CTX *context, struct player *player, const uv_buf_t *buf, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;

	uint8_t type;
	uint16_t len;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	packet = talloc_zero(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	if (packet_read_header(buf, &type, &len) < 0) {
		_ERROR("%s: failed to read header from socket.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	packet->type = type;
	packet->len = len;
	packet->player = player;

	//TODO: Packet sanity checks.

	*out_packet = talloc_steal(context, packet);
	ret = 0;

out:
	talloc_free(temp_context);

	return ret;
}

void packet_write_header(uint8_t type, uint16_t len, uv_buf_t *buf, int *pos)
{
	*(uint16_t *)buf->base = len;
	*pos += sizeof(uint16_t);
	buf->base[*pos] = type;
}