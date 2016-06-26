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
#include "../game.h"

#include "../binary_writer.h"
#include "../binary_reader.h"

#include "../util.h"

int player_info_handle(struct player *player, struct packet *packet)
{
	struct player_info *player_info = (struct player_info *)packet->data;
	
	player->name = talloc_strdup(player, player_info->name);
	
	player->stats.difficulty = player_info->difficulty;
	player->stats.eye_colour = player_info->eye_colour;
	player->stats.hair = player_info->hair;
	player->stats.hair_colour = player_info->hair_colour;
	player->stats.hair_dye = player_info->hair_dye;
	player->stats.hide_misc = player_info->hide_misc;
	player->stats.hide_visuals = player_info->hide_visuals;
	player->stats.hide_visuals2 = player_info->hide_visuals2;
	
	printf("%s (%s) has joined to slot %d.\n", player->name, player->remote_addr, player->id);
	
	return 0;
}

int player_info_new(TALLOC_CTX *ctx, const struct player *player, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct player_info *player_info;
	char *name_copy;

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

	packet->type = PACKET_TYPE_PLAYER_INFO;
	packet->len = PACKET_HEADER_SIZE;
	packet->data = NULL;
	
	player_info = talloc(temp_context, struct player_info);
	if (player_info == NULL) {
		_ERROR("%s: out of memory allocating player_info structure.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	name_copy = talloc_strdup(temp_context, player->name);
	if (name_copy == NULL) {
		_ERROR("%s: out of memory copying name to player_info struct\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	player_info->id = player->id;
	player_info->difficulty = player->stats.difficulty;
	player_info->eye_colour = player->stats.eye_colour;
	player_info->hair = player->stats.hair;
	player_info->hair_colour = player->stats.hair_colour;
	player_info->hair_dye = player->stats.hair_dye;
	player_info->hide_misc = player->stats.hide_misc;
	player_info->hide_visuals = player->stats.hide_visuals;
	player_info->hide_visuals2 = player->stats.hide_visuals2;
	player_info->pants_colour = player->stats.pants_colour;
	player_info->shirt_colour = player->stats.shirt_colour;
	player_info->shoe_colour = player->stats.shoe_colour;
	player_info->skin_colour = player->stats.skin_colour;
	player_info->skin_variant = player->stats.skin_variant;
	player_info->under_shirt_colour = player->stats.under_shirt_colour;
	
	player_info->name = talloc_steal(player_info, name_copy);
	
	packet->data = talloc_steal(packet, player_info);
	
	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int player_info_read(struct packet *packet)
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

	player_info->id = packet->data_buffer[pos++];
	player_info->skin_variant = packet->data_buffer[pos++];
	player_info->hair = packet->data_buffer[pos++];

	binary_reader_read_string_buffer(packet->data_buffer, pos, &name_len, &name);

	player_info->name = talloc_size(player_info, name_len + 1);
	memcpy(player_info->name, name, name_len);
	player_info->name[name_len] = '\0';

	pos += name_len + 1;
	
	player_info->hair_dye = packet->data_buffer[pos++];
	player_info->hide_visuals = packet->data_buffer[pos++];
	player_info->hide_visuals2 = packet->data_buffer[pos++];
	player_info->hide_misc = packet->data_buffer[pos++];

	player_info->hair_colour = *(struct colour *)(packet->data_buffer + pos);
	pos += sizeof(struct colour);
	player_info->skin_colour = *(struct colour *)(packet->data_buffer + pos);
	pos += sizeof(struct colour);
	player_info->eye_colour = *(struct colour *)(packet->data_buffer + pos);
	pos += sizeof(struct colour);
	player_info->shirt_colour = *(struct colour *)(packet->data_buffer + pos);
	pos += sizeof(struct colour);
	player_info->under_shirt_colour = *(struct colour *)(packet->data_buffer + pos);
	pos += sizeof(struct colour);
	player_info->pants_colour = *(struct colour *)(packet->data_buffer + pos);
	pos += sizeof(struct colour);
	player_info->shoe_colour = *(struct colour *)(packet->data_buffer + pos);
	pos += sizeof(struct colour);

	player_info->difficulty = packet->data_buffer[pos];

	packet->data = (void *)talloc_steal(packet, player_info);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

int player_info_write(const struct game *game, struct packet *packet)
{
	int pos = 0;
	struct player_info *player_info = (struct player_info *)packet->data;
	
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->id);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->skin_variant);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->hair);
	pos += binary_writer_write_string(packet->data_buffer + pos, player_info->name);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->hair_dye);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->hide_visuals);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->hide_visuals2);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->hide_misc);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->hair_colour);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->skin_colour);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->eye_colour);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->shirt_colour);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->under_shirt_colour);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->pants_colour);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->shoe_colour);
	pos += binary_writer_write_value(packet->data_buffer + pos, player_info->difficulty);
	
	return pos;
}