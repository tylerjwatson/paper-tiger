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

#include "client_uuid.h"
#include "../game.h"
#include "../binary_reader.h"
#include "../player.h"
#include "../util.h"
#include "../packet.h"

int client_uuid_handle(struct player *player, struct packet *packet)
{
	struct client_uuid *client_uuid = (struct client_uuid *)packet->data;

	/*
	 * No need to copy here, just reparent the UUID field from the packet data
	 * to the player structure since we aren't going to use it anywhere else.
	 */
	
	player->uuid = talloc_steal(player, client_uuid->uuid);

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

int client_uuid_read(struct packet *packet)
{
	int ret = -1, pos = 0, uuid_len = 0;
	TALLOC_CTX *temp_context;
	struct client_uuid *client_uuid;

	char *uuid;
	char *uuid_copy;

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

	binary_reader_read_string_buffer(packet->data_buffer, pos, &uuid_len, &uuid);

	uuid_copy = talloc_size(temp_context, uuid_len + 1);
	if (uuid_copy == NULL) {
		_ERROR("%s: out of memory allocating uuid.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	memcpy(uuid_copy, uuid, uuid_len);
	uuid_copy[uuid_len] = '\0';

	client_uuid->uuid = talloc_steal(client_uuid, uuid_copy);
	packet->data = (void *)talloc_steal(packet, client_uuid);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}