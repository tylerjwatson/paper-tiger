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

#include "player.h"
#include "game.h"
#include "util.h"

static void __player_handle_close(uv_handle_t *handle)
{
	talloc_free(handle);
}

static int __player_destructor(struct player *player)
{
	_ERROR("%s (%d) has disconnected.\n", player->name, player->remote_addr);
	
	bitmap_clear(player->game->player_slots, player->id);
	player->game->players[player->id] = NULL;
}

int player_new(TALLOC_CTX *context, const struct game *game, int id, struct player **out_player)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct player *player;
	
	if ((temp_context = talloc_new(NULL)) == NULL) {
		_ERROR("%s: allocating temporary talloc context failed.\n", __FUNCTION__);
		return -1;
	}
	
	if ((player = talloc_zero(temp_context, struct player)) == NULL) {
		_ERROR("%s: allocating player object failed.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}
	
	player->id = id;
	player->game = (struct game *)game;
	talloc_set_destructor(player, __player_destructor);
	
	*out_player = talloc_steal(context, player);
	ret = 0;
	
out:
	talloc_free(temp_context);
	return ret;
}

void player_close(struct player *player)
{
	if (player->handle != NULL) {
		/*
		 * If the TCP handle is connected, the player's memory cannot
		 * be freed until the player handle close callback has been called,
		 * only then may the memory claimed by the TCP handle be freed.
		 */
		uv_close((uv_handle_t *)player->handle, __player_handle_close);
		
		/*
		 * Re-parent the TCP handle's memory underneath the game context whilst
		 * it is still closing.  The memory will be freed once the handle close
		 * callback gets called.  This way the player structure can still get
		 * deallocated and exchanged to null whilst the handle is still being
		 * closed by the loop.
		 */
		talloc_steal(player->game, player->handle);
	}

	talloc_free(player);
}