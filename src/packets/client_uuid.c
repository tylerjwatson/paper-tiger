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

#include <string.h>

#include "client_uuid.h"
#include "../game.h"
#include "../binary_reader.h"
#include "../util.h"
#include "../packet.h"

int client_uuid_handle(struct player *player, struct packet *packet)
{
	
	return 0;
}

int client_uuid_new(TALLOC_CTX *ctx, const struct player *player, const char *uuid, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct client_uuid *client_uuid;
	
	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_CLIENT_UUID);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet type %d\n", __FUNCTION__, PACKET_TYPE_CLIENT_UUID);
		ret = -ENOMEM;
		goto out;
	}

	client_uuid = talloc(temp_context, struct client_uuid);
	if (client_uuid == NULL) {
		_ERROR("%s: out of memory allocating client_uuid.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	packet->type = PACKET_TYPE_CLIENT_UUID;
	packet->len = PACKET_HEADER_SIZE + PACKET_LEN_CLIENT_UUID;

	client_uuid->uuid = talloc_strdup(client_uuid, uuid);
	
	packet->data = (void *)talloc_steal(packet, client_uuid);

	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int client_uuid_read(struct packet *packet, const uv_buf_t *buf)
{
	int ret = -1, pos = 0, uuid_len = 0;
	TALLOC_CTX *temp_context;
	struct client_uuid *client_uuid;

	char *uuid;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for player info.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	client_uuid = talloc_zero(temp_context, struct client_uuid);
	if (client_uuid == NULL) {
		_ERROR("%s: out of memory allocating player info.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	binary_reader_read_string_buffer(buf->base, pos, &uuid_len, &uuid);

	client_uuid->uuid = talloc_strdup(client_uuid, uuid);

	packet->data = (void *)talloc_steal(packet, client_uuid);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}