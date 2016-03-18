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

#include "connect_request.h"
#include "../server.h"
#include "../game.h"
#include "../binary_reader.h"
#include "../util.h"

#include "continue_connecting.h"

int connect_request_read(struct packet *packet, const uv_buf_t *buf)
{
	TALLOC_CTX *temp_context;
	int ret = -1, pos = 0, str_len;
	struct connect_request *connect_request;
	char *protocol_version;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for connect request.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	connect_request = talloc_zero(temp_context, struct connect_request);
	if (connect_request == NULL) {
		_ERROR("%s: out of memory allocating connect request.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	binary_reader_read_7bit_int(buf->base, &pos, &str_len);

	protocol_version = talloc_size(temp_context, str_len + 1);
	if (protocol_version == NULL) {
		_ERROR("%s: out of memory copying protocol version to packet.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	memcpy(protocol_version, buf->base + pos, str_len);

	protocol_version[str_len] = '\0';

	connect_request->protocol_version = talloc_steal(connect_request, protocol_version);

	connect_request->packet = packet;
	packet->data = (void *)talloc_steal(packet, connect_request);
	
	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int connect_request_handle(const struct player *player, struct packet *packet)
{
	struct connect_request *req = (struct connect_request *)packet->data;
	struct packet *continue_connecting;

	char target_version[12];

	sprintf(target_version, "Terraria%d", GAME_PROTOCOL_VERSION);

	if (strcmp(req->protocol_version, target_version) == 0) {
		
		if (continue_connecting_new((struct player *)player, player, &continue_connecting) < 0) {
			_ERROR("%s: out of memory sending packet.\n", __FUNCTION__);
			goto error;
		}

		server_send_packet(player, (const struct packet *)continue_connecting);
		talloc_free(continue_connecting);
	}
	else {
		_ERROR("%s: disconnected slot 0 for incompatible version.\n", __FUNCTION__);
	}

	return 0;
error:
	return -1;
}