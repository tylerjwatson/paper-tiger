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

#include <string.h>

#include "continue_connecting.h"
#include "../game.h"
#include "../player.h"
#include "../packet.h"
#include "../binary_reader.h"
#include "../util.h"

int continue_connecting_new(TALLOC_CTX *ctx, uint8_t id, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct continue_connecting *continue_connecting;
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

	continue_connecting = talloc(temp_context, struct continue_connecting);
	if (continue_connecting == NULL) {
		_ERROR("%s: out of memory allocating continue connecting structure\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	packet->type = PACKET_TYPE_CONTINUE_CONNECTING;
	packet->len = PACKET_HEADER_SIZE + 1;

	continue_connecting->id = id;

	packet->data = talloc_steal(packet, continue_connecting);
	*out_packet = talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int continue_connecting_write(const struct game *game, const struct packet *packet, uv_buf_t buf)
{
	const struct continue_connecting *data = (const struct continue_connecting *)packet->data;
	buf.base[0] = data->id;

	return 1;
}
