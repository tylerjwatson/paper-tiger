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

#include "continue_connecting.h"
#include "../game.h"
#include "../binary_reader.h"
#include "../util.h"

int continue_connecting_new(TALLOC_CTX *ctx, const struct player *player, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_CONTINUE_CONNECTING);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet %d\n", __FUNCTION__, PACKET_TYPE_CONTINUE_CONNECTING);
		ret = -ENOMEM;
		goto out;
	}

	/*
	 * Packet has no payload.
	 */

	packet->type = PACKET_TYPE_CONTINUE_CONNECTING;
	packet->len = PACKET_HEADER_SIZE;
	packet->data = NULL;
	packet->player = (struct player *)player;

	*out_packet = talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int continue_connecting_write(TALLOC_CTX *context, struct packet *packet, uv_buf_t *buf)
{
	*buf = uv_buf_init(talloc_size(context, PACKET_LEN_CONTINUE_CONNECTING), PACKET_LEN_CONTINUE_CONNECTING);

	buf->base[0] = packet->player->id;

	return 0;
}