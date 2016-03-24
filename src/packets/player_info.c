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

#include "player_info.h"

#include "../packet.h"
#include "../player.h"


#include "../binary_reader.h"
#include "../util.h"

int player_info_handle(struct player *player, struct packet *packet)
{
	struct player_info *player_info = (struct player_info *)packet->data;
	
	player->name = talloc_strdup(player, player_info->name);
	
	printf("%s (%s) has joined to slot %d.\n", player->name, player->remote_addr, player->id);
	
	return 0;
}

int player_info_new(TALLOC_CTX *ctx, const struct player *player, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_PLAYER_INFO);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet %d\n", __FUNCTION__, PACKET_TYPE_PLAYER_INFO);
		ret = -ENOMEM;
		goto out;
	}

	/*
	 * Packet has no payload.
	 */

	packet->type = PACKET_TYPE_PLAYER_INFO;
	packet->len = PACKET_HEADER_SIZE;
	packet->data = NULL;

	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int player_info_read(struct packet *packet, const uv_buf_t *buf)
{
	int ret = -1, pos = 0, name_len = 0;
	TALLOC_CTX *temp_context;
	struct player_info *player_info;

	char *name;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for player info.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	player_info = talloc_zero(temp_context, struct player_info);
	if (player_info == NULL) {
		_ERROR("%s: out of memory allocating player info.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	player_info->id = buf->base[pos++];
	player_info->skin_variant = buf->base[pos++];
	player_info->hair = buf->base[pos++];

	binary_reader_read_string_buffer(buf->base, pos, &name_len, &name);

	player_info->name = talloc_size(player_info, name_len + 1);
	memcpy(player_info->name, name, name_len);
	player_info->name[name_len] = '\0';

	pos += name_len + 1;
	
	player_info->hair_dye = buf->base[pos++];
	player_info->hide_visuals = buf->base[pos++];
	player_info->hide_visuals2 = buf->base[pos++];
	player_info->hide_misc = buf->base[pos++];

	player_info->hair_colour = *(struct colour *)(buf->base + pos);
	pos += sizeof(struct colour);
	player_info->skin_colour = *(struct colour *)(buf->base + pos);
	pos += sizeof(struct colour);
	player_info->eye_colour = *(struct colour *)(buf->base + pos);
	pos += sizeof(struct colour);
	player_info->shirt_colour = *(struct colour *)(buf->base + pos);
	pos += sizeof(struct colour);
	player_info->under_shirt_colour = *(struct colour *)(buf->base + pos);
	pos += sizeof(struct colour);
	player_info->pants_colour = *(struct colour *)(buf->base + pos);
	pos += sizeof(struct colour);
	player_info->shoe_colour = *(struct colour *)(buf->base + pos);
	pos += sizeof(struct colour);

	player_info->difficulty = buf->base[pos];

	packet->data = (void *)talloc_steal(packet, player_info);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}