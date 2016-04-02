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

#include "chat_message.h"
#include "../game.h"
#include "../console.h"
#include "../binary_reader.h"
#include "../binary_writer.h"
#include "../player.h"
#include "../util.h"
#include "../packet.h"
#include "../server.h"
#include "../colour.h"

int chat_message_handle(struct player *player, struct packet *packet)
{
	struct chat_message *chat_message = (struct chat_message *)packet->data;

	console_vsprintf(player->game->console, "<\033[33;1m%s\033[0m> %s\n", player->name, chat_message->message);
	
	/*
	 * The chat message can be re-broadcasted to everyone else, but
	 * must have the ID of the originating player set in the message.
	 */
	chat_message->id = player->id;
	server_broadcast_packet(player->game->server, packet);

	return 0;
}

int chat_message_new(TALLOC_CTX *ctx, const struct player *player, const struct colour colour,
					 const char *message, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct chat_message *chat_message;
	
	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_CHAT_MESSAGE);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet type %d\n", __FUNCTION__, PACKET_TYPE_CHAT_MESSAGE);
		ret = -ENOMEM;
		goto out;
	}

	chat_message = talloc(temp_context, struct chat_message);
	if (chat_message == NULL) {
		_ERROR("%s: out of memory allocating chat_message.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	packet->type = PACKET_TYPE_CHAT_MESSAGE;
	packet->len = PACKET_HEADER_SIZE + (uint16_t)strlen(message) + binary_writer_7bit_len(strlen(message));

	chat_message->id = player->id;
	chat_message->colour = colour;
	chat_message->message = talloc_strdup(chat_message, message);
	if (chat_message->message == NULL) {
		_ERROR("%s: out of memory copying chat message to packet.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	packet->data = (void *)talloc_steal(packet, chat_message);

	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;

out:
	talloc_free(temp_context);

	return ret;
}

int chat_message_read(struct packet *packet, const uv_buf_t *buf)
{
	int ret = -1, pos = 0, message_len = 0;
	TALLOC_CTX *temp_context;
	struct chat_message *chat_message;

	char *message;
	char *message_copy;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for player info.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	chat_message = talloc_zero(temp_context, struct chat_message);
	if (chat_message == NULL) {
		_ERROR("%s: out of memory allocating player info.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	chat_message->id = buf->base[pos++];
	chat_message->colour = *(struct colour *)(buf->base + pos);
	pos += sizeof(struct colour);

	binary_reader_read_string_buffer(buf->base, pos, &message_len, &message);

	message_copy = talloc_size(temp_context, message_len + 1);
	if (message_copy == NULL) {
		_ERROR("%s: out of memory allocating chat message from packet.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	memcpy(message_copy, message, message_len);
	message_copy[message_len] = '\0';

	chat_message->message = talloc_steal(chat_message, message_copy);
	packet->data = (void *)talloc_steal(packet, chat_message);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int chat_message_write(const struct game *game, const struct packet *packet, uv_buf_t buffer)
{
	struct chat_message *chat_message = (struct chat_message *)packet->data;
	int pos = 0;

	pos += binary_writer_write_value(buffer.base + pos, chat_message->id);
	pos += binary_writer_write_value(buffer.base + pos, chat_message->colour);
	pos += binary_writer_write_string(buffer.base + pos, chat_message->message);

	return pos;
}