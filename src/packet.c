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

#include "player.h"
#include "packet.h"
#include "game.h"
#include "util.h"

#include "packets/world_info.h"
#include "packets/connect_request.h"
#include "packets/continue_connecting.h"
#include "packets/continue_connecting2.h"
#include "packets/player_info.h"
#include "packets/client_uuid.h"
#include "packets/player_hp.h"
#include "packets/player_mana.h"
#include "packets/disconnect.h"
#include "packets/get_section.h"
#include "packets/tile_section.h"
#include "packets/section_tile_frame.h"
#include "packets/connection_complete.h"
#include "packets/inventory_slot.h"
#include "packets/chat_message.h"
#include "packets/status.h"

static struct packet_handler packet_handlers[] = {
	{ .type = PACKET_TYPE_CONNECT_REQUEST, .read_func = connect_request_read, .handle_func = connect_request_handle, .write_func = NULL },
	{ .type = PACKET_TYPE_CONTINUE_CONNECTING, .read_func = NULL, .handle_func = NULL, .write_func = continue_connecting_write },
	{ .type = PACKET_TYPE_PLAYER_INFO, .read_func = player_info_read, .handle_func = player_info_handle, .write_func = NULL },
	{ .type = PACKET_TYPE_INVENTORY_SLOT, .read_func = inventory_slot_read, .handle_func = inventory_slot_handle, .write_func = NULL },
	{ .type = PACKET_TYPE_CONTINUE_CONNECTING2, .read_func = NULL, .handle_func = continue_connecting2_handle, .write_func = NULL },
	{ .type = PACKET_TYPE_WORLD_INFO, .read_func = NULL, .handle_func = NULL, .write_func = world_info_write },
	{ .type = PACKET_TYPE_GET_SECTION, .read_func = get_section_read, .handle_func = get_section_handle, .write_func = NULL },
	{ .type = PACKET_TYPE_STATUS, .read_func = NULL, .handle_func = NULL, .write_func = status_write },
	{ .type = PACKET_TYPE_TILE_SECTION, .read_func = NULL, .handle_func = NULL, .write_func = tile_section_write },
	{ .type = PACKET_TYPE_SECTION_TILE_FRAME, .read_func = NULL, .handle_func = NULL, .write_func = section_tile_frame_write },
	{ .type = PACKET_TYPE_CHAT_MESSAGE, .read_func = chat_message_read, .handle_func = chat_message_handle, .write_func = chat_message_write }, 
	{ .type = PACKET_TYPE_PLAYER_HP, .read_func = player_hp_read, .handle_func = player_hp_handle, .write_func = NULL },
	{ .type = PACKET_TYPE_PLAYER_MANA, .read_func = player_mana_read, .handle_func = player_mana_handle, .write_func = NULL },
	{ .type = PACKET_TYPE_CONNECTION_COMPLETE, .read_func = NULL, .handle_func = NULL, .write_func = connection_complete_write },
	{ .type = PACKET_TYPE_CLIENT_UUID, .read_func = client_uuid_read, .handle_func = client_uuid_handle, .write_func = NULL },
	{ .type = PACKET_TYPE_DISCONNECT, .read_func = disconnect_read, .handle_func = disconnect_handle, .write_func = disconnect_write },
	{ 0x00, NULL, NULL, NULL }
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
	//packet->player = player;

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