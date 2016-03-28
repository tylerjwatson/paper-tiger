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

#include "inventory_slot.h"

#include "../item.h"
#include "../game.h"
#include "../binary_reader.h"
#include "../player.h"
#include "../util.h"
#include "../packet.h"

int inventory_slot_handle(struct player *player, struct packet *packet)
{
	struct item_slot *inventory_slot = (struct item_slot *)packet->data;
	struct item_slot player_slot = player->inventory[inventory_slot->slot_id];
	
	player_slot.net_id = inventory_slot->net_id;
	player_slot.prefix = inventory_slot->prefix;
	player_slot.slot_id = inventory_slot->slot_id;
	player_slot.stack = inventory_slot->stack;
	
	return 0;
}

int inventory_slot_new(TALLOC_CTX *ctx, const struct player *player, const struct item_slot slot, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct item_slot *item_slot;
	
	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_INVENTORY_SLOT);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet type %d\n", __FUNCTION__, PACKET_TYPE_INVENTORY_SLOT);
		ret = -ENOMEM;
		goto out;
	}

	item_slot = talloc(temp_context, struct item_slot);
	if (item_slot == NULL) {
		_ERROR("%s: out of memory allocating item_slot.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	packet->type = PACKET_TYPE_INVENTORY_SLOT;
	packet->len = PACKET_HEADER_SIZE + PACKET_LEN_INVENTORY_SLOT;

	memcpy(item_slot, &slot, sizeof(slot));

	packet->data = (void *)talloc_steal(packet, item_slot);
	
	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int inventory_slot_read(struct packet *packet, const uv_buf_t *buf)
{
	int ret = -1, pos = 0;
	TALLOC_CTX *temp_context;
	struct item_slot *item_slot;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for player info.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	item_slot = talloc_zero(temp_context, struct item_slot);
	if (item_slot == NULL) {
		_ERROR("%s: out of memory allocating player info.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out; 
	}

	item_slot->id = buf->base[pos++];
	item_slot->slot_id = buf->base[pos++];
	item_slot->stack = *(int16_t *)(buf->base + pos);
	pos += sizeof(uint16_t);
	item_slot->prefix = buf->base[pos++];
	item_slot->net_id = *(int16_t *)(buf->base + pos);

	packet->data = (void *)talloc_steal(packet, item_slot);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}