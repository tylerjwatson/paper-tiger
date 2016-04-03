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

#include "status.h"

#include "../game.h"
#include "../console.h"
#include "../binary_reader.h"
#include "../binary_writer.h"
#include "../player.h"
#include "../util.h"
#include "../packet.h"
#include "../server.h"

int status_new(TALLOC_CTX *ctx, const struct player *player, uint32_t duration,
			   const char *message, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct status *status;
	
	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_STATUS);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet type %d\n", __FUNCTION__, PACKET_TYPE_STATUS);
		ret = -ENOMEM;
		goto out;
	}

	status = talloc(temp_context, struct status);
	if (status == NULL) {
		_ERROR("%s: out of memory allocating status.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	packet->type = PACKET_TYPE_STATUS;
	packet->len = PACKET_HEADER_SIZE + strlen(message) + binary_writer_7bit_len(strlen(message));

	status->message_duration = duration;
	
	status->message = talloc_strdup(status, message);
	if (status->message == NULL) {
		_ERROR("%s: out of memory copying message to packet.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	packet->data = (void *)talloc_steal(packet, status);

	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;

out:
	talloc_free(temp_context);

	return ret;
}

int status_write(const struct game *game, const struct packet *packet, uv_buf_t buffer)
{
	struct status *status = (struct status *)packet->data;
	int pos = 0;

	pos += binary_writer_write_value(buffer.base + pos, status->message_duration);
	pos += binary_writer_write_string(buffer.base + pos, status->message);

	return pos;
}