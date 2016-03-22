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
#include <stdbool.h> 


#include "world_info.h"
#include "../world.h"
#include "../game.h"
#include "../packet.h"
#include "../player.h"


#include "../binary_reader.h"
#include "../binary_writer.h"
#include "../util.h"

#define ARRAY_SIZEOF(a) sizeof(a)/sizeof(a[0])

static inline void bit_toggle(uint8_t *bit, int n, bool value)
{
	*bit ^= (-value ^ *bit) & (1 << n);
}

static void __fill_world_info(struct world *world, struct world_info *world_info)
{
	world_info->time = world->temp_time;
	bit_toggle(&world_info->day_info, 0, world->temp_day_time);
	bit_toggle(&world_info->day_info, 1, world->temp_blood_moon);
	bit_toggle(&world_info->day_info, 2, world->temp_eclipse);
	world_info->moon_phase = world->temp_moon_phase;
	world_info->max_tile_x = world->max_tiles_x;
	world_info->max_tile_y = world->max_tiles_y;
	world_info->spawn_tile_x = world->spawn_tile.x;
	world_info->spawn_tile_y = world->spawn_tile.y;
	world_info->world_surface = world->world_surface;
	world_info->rock_layer = world->rock_layer;
	world_info->world_id = world->worldID;
	binary_writer_write_string(world_info, world->world_name, &world_info->world_name);
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
	world_info->num_clouds = world->num_clouds;
	
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
	
	bit_toggle(&world_info->flags_1, 0, world->flags.shadow_orb_smashed);
	bit_toggle(&world_info->flags_1, 1, world->flags.downed_boss_1);
	bit_toggle(&world_info->flags_1, 2, world->flags.downed_boss_2);
	bit_toggle(&world_info->flags_1, 3, world->flags.downed_boss_3);
	bit_toggle(&world_info->flags_1, 4, world->flags.hard_mode);
	bit_toggle(&world_info->flags_1, 5, world->flags.downed_clowns);
	bit_toggle(&world_info->flags_1, 6, 1 /*SSO Support*/);
	bit_toggle(&world_info->flags_1, 7, world->flags.downed_plant);
	
	bit_toggle(&world_info->flags_2, 0, world->flags.downed_mech_1);
	bit_toggle(&world_info->flags_2, 1, world->flags.downed_mech_2);
	bit_toggle(&world_info->flags_2, 2, world->flags.downed_mech_3);
	bit_toggle(&world_info->flags_2, 3, world->flags.downed_mech_any);
	bit_toggle(&world_info->flags_2, 4, world->cloud_bg_active > 1.);
	bit_toggle(&world_info->flags_2, 5, world->flags.crimson);
	bit_toggle(&world_info->flags_2, 6, 0 /* Pumpkin moon */);
	bit_toggle(&world_info->flags_2, 7, 0 /* Snow moon */);
	
	bit_toggle(&world_info->flags_3, 0, world->expert_mode);
	bit_toggle(&world_info->flags_3, 1, world->fast_forward_time);
	bit_toggle(&world_info->flags_3, 2, 0 /* Slime rain */);
	bit_toggle(&world_info->flags_3, 3, world->flags.downed_slime_king);
	bit_toggle(&world_info->flags_3, 4, world->flags.downed_queen_bee);
	bit_toggle(&world_info->flags_3, 5, world->flags.downed_fishron);
	bit_toggle(&world_info->flags_3, 6, world->flags.downed_martians);
	bit_toggle(&world_info->flags_3, 7, world->flags.downed_ancient_cultist);
	
	bit_toggle(&world_info->flags_3, 0, world->expert_mode);
	bit_toggle(&world_info->flags_3, 1, world->fast_forward_time);
	bit_toggle(&world_info->flags_3, 2, 0 /* Slime rain */);
	bit_toggle(&world_info->flags_3, 3, world->flags.downed_slime_king);
	bit_toggle(&world_info->flags_3, 4, world->flags.downed_queen_bee);
	bit_toggle(&world_info->flags_3, 5, world->flags.downed_fishron);
	bit_toggle(&world_info->flags_3, 6, world->flags.downed_martians);
	bit_toggle(&world_info->flags_3, 7, world->flags.downed_ancient_cultist);
	
	bit_toggle(&world_info->flags_4, 0, world->flags.downed_moonlord);
	bit_toggle(&world_info->flags_4, 1, world->flags.downed_halloween_king);
	bit_toggle(&world_info->flags_4, 2, world->flags.downed_halloween_tree);
	bit_toggle(&world_info->flags_4, 3, world->flags.downed_christmas_ice_queen);
	bit_toggle(&world_info->flags_4, 4, world->flags.downed_christmas_santank);
	bit_toggle(&world_info->flags_4, 5, world->flags.downed_christmas_tree);
	bit_toggle(&world_info->flags_4, 6, world->flags.downed_golem);
	bit_toggle(&world_info->flags_4, 7, 0 /* Not used */);

	world_info->invasion_type = world->invasion_type;
	world_info->lobby_id = 0;
}

int world_info_new(TALLOC_CTX *ctx, const struct player *player, struct packet **out_packet)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct packet *packet;
	struct world_info *world_info;
	struct world *world = player->game->world;

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

	world_info = talloc(temp_context, struct world_info);
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