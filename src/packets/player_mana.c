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

#include "player_mana.h"

#include "../packet.h"
#include "../player.h"

#include "../binary_reader.h"
#include "../util.h"

int player_mana_handle(struct player *player, struct packet *packet)
{
	struct player_mana *player_mana = (struct player_mana *)packet->data;

	player->mana = player_mana->mana;
	player->mana_max = player_mana->mana_max;

	return 0;
}

int player_mana_new(TALLOC_CTX *ctx, const struct player *player, uint16_t life, uint16_t life_max, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_PLAYER_MANA);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet %d\n", __FUNCTION__, PACKET_TYPE_PLAYER_MANA);
		ret = -ENOMEM;
		goto out;
	}

	packet->type = PACKET_TYPE_PLAYER_MANA;
	packet->len = PACKET_HEADER_SIZE;
	packet->data = NULL;

	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int player_mana_read(struct packet *packet)
{
	int ret = -1, pos = 0;
	TALLOC_CTX *temp_context;
	struct player_mana *player_mana;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for player mana.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	player_mana = talloc_zero(temp_context, struct player_mana);
	if (player_mana == NULL) {
		_ERROR("%s: out of memory allocating player mana packet.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	player_mana->id = packet->data_buffer[pos++];
	player_mana->mana = *(uint16_t *)(packet->data_buffer + pos);
	pos += sizeof(uint16_t);
	player_mana->mana_max = *(uint16_t *)(packet->data_buffer + pos);

	packet->data = talloc_steal(packet, player_mana);
	
	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}