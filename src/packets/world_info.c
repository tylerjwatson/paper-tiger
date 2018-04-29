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
#include <stdbool.h>

#include "world_info.h"

#include "../world.h"
#include "../game.h"
#include "../packet.h"
#include "../player.h"

#include "../talloc/talloc.h"
#include "../binary_reader.h"
#include "../binary_writer.h"
#include "../util.h"

#define ARRAY_SIZEOF(a) sizeof(a)/sizeof(a[0])

static int __fill_world_info_buffer(struct world_info *world_info, uint8_t *buffer)
{
	int pos = 0;
	
	pos += binary_writer_write_value(buffer + pos, world_info->time);
	pos += binary_writer_write_value(buffer + pos, world_info->day_info);
	pos += binary_writer_write_value(buffer + pos, world_info->moon_phase);
	pos += binary_writer_write_value(buffer + pos, world_info->max_tile_x);
	pos += binary_writer_write_value(buffer + pos, world_info->max_tile_y);
	pos += binary_writer_write_value(buffer + pos, world_info->spawn_tile_x);
	pos += binary_writer_write_value(buffer + pos, world_info->spawn_tile_y);
	pos += binary_writer_write_value(buffer + pos, world_info->world_surface);
	pos += binary_writer_write_value(buffer + pos, world_info->rock_layer);
	pos += binary_writer_write_value(buffer + pos, world_info->world_id);
	pos += binary_writer_write_string(buffer + pos, world_info->world_name);
	pos += binary_writer_write_value(buffer + pos, world_info->moon_type);
	pos += binary_writer_write_value(buffer + pos, world_info->bg_tree);
	pos += binary_writer_write_value(buffer + pos, world_info->bg_corrupt);
	pos += binary_writer_write_value(buffer + pos, world_info->bg_jungle);
	pos += binary_writer_write_value(buffer + pos, world_info->bg_snow);
	pos += binary_writer_write_value(buffer + pos, world_info->bg_hallow);
	pos += binary_writer_write_value(buffer + pos, world_info->bg_crimson);
	pos += binary_writer_write_value(buffer + pos, world_info->bg_desert);
	pos += binary_writer_write_value(buffer + pos, world_info->bg_ocean);
	pos += binary_writer_write_value(buffer + pos, world_info->style_ice_back);
	pos += binary_writer_write_value(buffer + pos, world_info->style_jungle_back);
	pos += binary_writer_write_value(buffer + pos, world_info->style_hell_back);
	pos += binary_writer_write_value(buffer + pos, world_info->wind_speed_set);
	pos += binary_writer_write_value(buffer + pos, world_info->num_clouds);
	
	for(int i = 0; i < ARRAY_SIZEOF(world_info->tree_x); i++) {
		pos += binary_writer_write_value(buffer + pos, world_info->tree_x[i]);
	}
	
	for(int i = 0; i < ARRAY_SIZEOF(world_info->tree_style); i++) {
		pos += binary_writer_write_value(buffer + pos, world_info->tree_style[i]);
	}
	
	for(int i = 0; i < ARRAY_SIZEOF(world_info->cave_back_x); i++) {
		pos += binary_writer_write_value(buffer + pos, world_info->cave_back_x[i]);
	}
	
	for(int i = 0; i < ARRAY_SIZEOF(world_info->cave_back_style); i++) {
		pos += binary_writer_write_value(buffer + pos, world_info->cave_back_style[i]);
	}
	
	pos += binary_writer_write_value(buffer + pos, world_info->max_raining);
	pos += binary_writer_write_value(buffer + pos, world_info->flags_1);
	pos += binary_writer_write_value(buffer + pos, world_info->flags_2);
	pos += binary_writer_write_value(buffer + pos, world_info->flags_3);
	pos += binary_writer_write_value(buffer + pos, world_info->flags_4);

	return pos;
}

static void __fill_world_info(struct world *world, struct world_info *world_info)
{
	world_info->time = (int)world->temp_time;
	if (world->temp_day_time) {
		BIT_SET(world_info->day_info, 0);
	}
	if (world->temp_blood_moon) {
		BIT_SET(world_info->day_info, 1);
	}
	if (world->temp_eclipse) {
		BIT_SET(world_info->day_info, 2);
	}
	world_info->moon_phase = world->temp_moon_phase;
	world_info->max_tile_x = world->max_tiles_x;
	world_info->max_tile_y = world->max_tiles_y;
	world_info->spawn_tile_x = world->spawn_tile.x;
	world_info->spawn_tile_y = world->spawn_tile.y;
	world_info->world_surface = (int16_t)world->world_surface;
	world_info->rock_layer = (int16_t)world->rock_layer;
	world_info->world_id = world->worldID;
	world_info->world_name = talloc_strdup(world_info, world->world_name);
	world_info->moon_type = world->moon_type;
	world_info->bg_tree = world->bg[0];
	world_info->bg_corrupt = world->bg[1];
	world_info->bg_jungle = world->bg[2];
	world_info->bg_snow = world->bg[3];
	world_info->bg_hallow = world->bg[4];
	world_info->bg_crimson = world->bg[5];
	world_info->bg_desert = world->bg[6];
	world_info->bg_ocean = world->bg[7];
	world_info->style_ice_back = world->ice_back_style;
	world_info->style_jungle_back = world->jungle_back_style;
	world_info->style_hell_back = world->hell_back_style;
	world_info->wind_speed_set = world->wind_speed;
	world_info->num_clouds = (uint8_t)world->num_clouds;

	for(int i = 0; i < ARRAY_SIZEOF(world->tree_x); i++) {
		world_info->tree_x[i] = world->tree_x[i];
	}

	for(int i = 0; i < ARRAY_SIZEOF(world->tree_style); i++) {
		world_info->tree_style[i] = world->tree_style[i];
	}

	for(int i = 0; i < ARRAY_SIZEOF(world->cave_back_x); i++) {
		world_info->cave_back_x[i] = world->cave_back_x[i];
	}

	for(int i = 0; i < ARRAY_SIZEOF(world->cave_back_style); i++) {
		world_info->cave_back_style[i] = world->cave_back_style[i];
	}

	world_info->max_raining = world->max_rain;

	if (world->flags.shadow_orb_smashed) {
		BIT_SET(world_info->flags_1, 0);
	}
	if (world->flags.downed_boss_1) {
		BIT_SET(world_info->flags_1, 1);
	}
	if (world->flags.downed_boss_2) {
		BIT_SET(world_info->flags_1, 2);
	}
	if (world->flags.downed_boss_3) {
		BIT_SET(world_info->flags_1, 3);
	}
	if (world->flags.hard_mode) {
		BIT_SET(world_info->flags_1, 4);
	}
	if (world->flags.downed_clowns) {
		BIT_SET(world_info->flags_1, 5);
	}
	if (1 /*SSO Support*/) {
		BIT_SET(world_info->flags_1, 6);
	}
	if (world->flags.downed_plant) {
		BIT_SET(world_info->flags_1, 7);
	}
	
	if (world->flags.downed_mech_1) {
		BIT_SET(world_info->flags_2, 0);
	}
	if (world->flags.downed_mech_2) {
		BIT_SET(world_info->flags_2, 1);
	}
	if (world->flags.downed_mech_3) {
		BIT_SET(world_info->flags_2, 2);
	}
	if (world->flags.downed_mech_any) {
		BIT_SET(world_info->flags_2, 3);
	}
	if ((world->cloud_bg_active > 1.)) {
		BIT_SET(world_info->flags_2, 4);
	}
	if (world->flags.crimson) {
		BIT_SET(world_info->flags_2, 5);
	}
	if (0 /* Pumpkin moon */) {
		BIT_SET(world_info->flags_2, 6);
	}
	if (0 /* Snow moon */) {
		BIT_SET(world_info->flags_2, 7);
	}

	if (world->expert_mode) {
		BIT_SET(world_info->flags_3, 0);
	}
	if (world->fast_forward_time) {
		BIT_SET(world_info->flags_3, 1);
	}
	if (0 /* Slime rain */) {
		BIT_SET(world_info->flags_3, 2);
	}
	if (world->flags.downed_slime_king) {
		BIT_SET(world_info->flags_3, 3);
	}
	if (world->flags.downed_queen_bee) {
		BIT_SET(world_info->flags_3, 4);
	}
	if (world->flags.downed_fishron) {
		BIT_SET(world_info->flags_3, 5);
	}
	if (world->flags.downed_martians) {
		BIT_SET(world_info->flags_3, 6);
	}
	if (world->flags.downed_ancient_cultist) {
		BIT_SET(world_info->flags_3, 7);
	}

	if (world->flags.downed_moonlord) {
		BIT_SET(world_info->flags_4, 0);
	}
	if (world->flags.downed_halloween_king) {
		BIT_SET(world_info->flags_4, 1);
	}
	if (world->flags.downed_halloween_tree) {
		BIT_SET(world_info->flags_4, 2);
	}
	if (world->flags.downed_christmas_ice_queen) {
		BIT_SET(world_info->flags_4, 3);
	}
	if (world->flags.downed_christmas_santank) {
		BIT_SET(world_info->flags_4, 4);
	}
	if (world->flags.downed_christmas_tree) {
		BIT_SET(world_info->flags_4, 5);
	}
	if (world->flags.downed_golem) {
		BIT_SET(world_info->flags_4, 6);
	}
	if (0 /* Not used */) {
		BIT_SET(world_info->flags_4, 7);
	}

	world_info->invasion_type = world->invasion_type;
	world_info->lobby_id = 0;
}

int world_info_write(const struct game *game, struct packet *packet)
{
	struct world_info *world_info = (struct world_info *)packet->data;
	int packet_len = __fill_world_info_buffer(world_info, packet->data_buffer);

	return packet_len;
}

int world_info_new(TALLOC_CTX *ctx, const struct player *player, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct world_info *world_info;
	struct world *world = &player->game->world;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for packet %d\n", __FUNCTION__, PACKET_TYPE_WORLD_INFO);
		ret = -ENOMEM;
		goto out;
	}

	packet = talloc(temp_context, struct packet);
	if (packet == NULL) {
		_ERROR("%s: out of memory allocating packet %d\n", __FUNCTION__, PACKET_TYPE_WORLD_INFO);
		ret = -ENOMEM;
		goto out;
	}

	world_info = talloc_zero(temp_context, struct world_info);
	if (world_info == NULL) {
		_ERROR("%s: out of memory allocating world info structure.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	__fill_world_info(world, world_info);

	packet->type = PACKET_TYPE_WORLD_INFO;
	packet->len = PACKET_HEADER_SIZE;
	packet->data = talloc_steal(packet, world_info);

	*out_packet = (struct packet *)talloc_steal(ctx, packet);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}
