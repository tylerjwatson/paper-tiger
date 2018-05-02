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
 *
 * paper-tiger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paper-tiger.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>

#include "game.h"
#include "packet.h"
#include "player.h"
#include "util.h"

#include "packets/chat_message.h"
#include "packets/client_uuid.h"
#include "packets/connect_request.h"
#include "packets/connection_complete.h"
#include "packets/continue_connecting.h"
#include "packets/continue_connecting2.h"
#include "packets/disconnect.h"
#include "packets/get_section.h"
#include "packets/inventory_slot.h"
#include "packets/player_hp.h"
#include "packets/player_info.h"
#include "packets/player_mana.h"
#include "packets/section_tile_frame.h"
#include "packets/status.h"
#include "packets/tile_section.h"
#include "packets/world_info.h"

static struct packet_handler packet_handlers[] = {
	{.type = PACKET_TYPE_CONNECT_REQUEST,
	 .read_func = connect_request_read,
	 .handle_func = connect_request_handle,
	 .write_func = NULL},
	{.type = PACKET_TYPE_CONTINUE_CONNECTING,
	 .read_func = NULL,
	 .handle_func = NULL,
	 .write_func = continue_connecting_write},
	{.type = PACKET_TYPE_PLAYER_INFO,
	 .read_func = player_info_read,
	 .handle_func = player_info_handle,
	 .write_func = player_info_write},
	{.type = PACKET_TYPE_INVENTORY_SLOT,
	 .read_func = inventory_slot_read,
	 .handle_func = inventory_slot_handle,
	 .write_func = NULL},
	{.type = PACKET_TYPE_CONTINUE_CONNECTING2,
	 .read_func = NULL,
	 .handle_func = continue_connecting2_handle,
	 .write_func = NULL},
	{.type = PACKET_TYPE_WORLD_INFO, .read_func = NULL, .handle_func = NULL, .write_func = world_info_write},
	{.type = PACKET_TYPE_GET_SECTION,
	 .read_func = get_section_read,
	 .handle_func = get_section_handle,
	 .write_func = NULL},
	{.type = PACKET_TYPE_STATUS, .read_func = NULL, .handle_func = NULL, .write_func = status_write},
	{.type = PACKET_TYPE_TILE_SECTION, .read_func = NULL, .handle_func = NULL, .write_func = tile_section_write_v2},
	{.type = PACKET_TYPE_SECTION_TILE_FRAME,
	 .read_func = NULL,
	 .handle_func = NULL,
	 .write_func = section_tile_frame_write},
	{.type = PACKET_TYPE_CHAT_MESSAGE,
	 .read_func = chat_message_read,
	 .handle_func = chat_message_handle,
	 .write_func = chat_message_write},
	{.type = PACKET_TYPE_PLAYER_HP, .read_func = player_hp_read, .handle_func = player_hp_handle, .write_func = NULL},
	{.type = PACKET_TYPE_PLAYER_MANA,
	 .read_func = player_mana_read,
	 .handle_func = player_mana_handle,
	 .write_func = NULL},
	{.type = PACKET_TYPE_CONNECTION_COMPLETE,
	 .read_func = NULL,
	 .handle_func = NULL,
	 .write_func = connection_complete_write},
	{.type = PACKET_TYPE_CLIENT_UUID,
	 .read_func = client_uuid_read,
	 .handle_func = client_uuid_handle,
	 .write_func = NULL},
	{.type = PACKET_TYPE_DISCONNECT,
	 .read_func = disconnect_read,
	 .handle_func = disconnect_handle,
	 .write_func = disconnect_write},
	{0x00, NULL, NULL, NULL}};

struct packet_handler *
packet_handler_for_type(uint8_t type)
{
	struct packet_handler *handler;

	for (handler = packet_handlers; handler->type != 0x00; handler++) {
		if (handler->type == type) {
			return handler;
		}
	}

	return NULL;
}

int
packet_read_header(const uv_buf_t *buf, uint8_t *out_type, uint16_t *out_len)
{
	if (buf->len < 3) {
		return -1;
	}

	*out_len = *(uint16_t *)buf->base;
	*out_type = buf->base[2];

	return 0;
}

int
packet_deserialize(const struct packet *packet)
{
	struct packet_handler *handler;
	packet_read_cb cb;

	handler = packet_handler_for_type(packet->type);
	if (handler == NULL) {
		_ERROR("%s: packet deserialize failed: no handler for type %d.\n", __FUNCTION__, packet->type);
		return -1;
	}

	cb = handler->read_func;
	if (cb == NULL) {
		_ERROR("%s: packet deserialize failed: no read handler for type %d.\n", __FUNCTION__, packet->type);
		return -1;
	}

	if (cb((struct packet *)packet) < 0) {
		_ERROR("%s: packet deserialize failed: error in read function for type %d.\n", __FUNCTION__, packet->type);
		return -1;
	}

	return 0;
}

int
packet_serialize(const struct game *game, struct packet *packet)
{
	struct packet_handler *handler;
	packet_write_cb cb;
	int len = 0;

	handler = packet_handler_for_type(packet->type);
	if (handler == NULL) {
		_ERROR("%s: serialize packet for type %d failed, no write function.\n", __FUNCTION__, packet->type);
		return -1;
	}

	cb = handler->write_func;
	if (cb == NULL) {
		_ERROR("%s: serialize packet for type %d failed, no write function.\n", __FUNCTION__, packet->type);
		return -1;
	}

	len = cb(game, packet);
	if (len < 0) {
		_ERROR("%s: serialize packet for type %d failed, write callback failed.\n", __FUNCTION__, packet->type);
		return -1;
	}

	packet->len = len + PACKET_HEADER_SIZE;

	return 0;
}

int
packet_init(struct packet *packet)
{
	memset(packet, 0, sizeof(*packet));

	return 0;
}

int
packet_recipient_all_online(const struct game *game, const struct packet *packet, int8_t ignore_id)
{
	int online_len = 0;
	uint8_t online_players[256];
	uint8_t id;

	online_len = game_online_players(game, online_players);

	for (int i = 0; i < online_len; i++) {
		id = online_players[i];

		bitmap_set((word_t *)packet->recipients, id);
	}

	if (ignore_id >= 0) {
		bitmap_clear((word_t *)packet->recipients, ignore_id);
	}

	return 0;
}
