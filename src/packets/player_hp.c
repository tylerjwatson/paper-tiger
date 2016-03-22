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

#include "player_hp.h"

#include "../packet.h"
#include "../player.h"

#include "../binary_reader.h"
#include "../util.h"

int player_hp_handle(struct player *player, struct packet *packet)
{
	struct player_hp *player_hp = (struct player_hp *)packet->data;
	
	player->life = player_hp->life;
	player->life_max = player_hp->life_max;

	return 0;
}

int player_hp_new(TALLOC_CTX *ctx, const struct player *player, uint16_t life, uint16_t life_max, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_PLAYER_HP);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet %d\n", __FUNCTION__, PACKET_TYPE_PLAYER_HP);
		ret = -ENOMEM;
		goto out;
	}

	packet->type = PACKET_TYPE_PLAYER_HP;
	packet->len = PACKET_HEADER_SIZE;
	packet->data = NULL;

	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int player_hp_read(struct packet *packet, const uv_buf_t *buf)
{
	int ret = -1, pos = 0;
	TALLOC_CTX *temp_context;
	struct player_hp *player_hp;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for player hp.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	player_hp = talloc_zero(temp_context, struct player_hp);
	if (player_hp == NULL) {
		_ERROR("%s: out of memory allocating player hp packet.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	player_hp->id = buf->base[pos++];
	player_hp->life = *(uint16_t *)(buf->base + pos);
	pos += sizeof(uint16_t);
	player_hp->life_max = *(uint16_t *)(buf->base + pos);

	packet->data = talloc_steal(packet, player_hp);
	
	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}