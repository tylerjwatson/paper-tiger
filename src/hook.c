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

#include "hook.h"

#include "game.h"
#include "player.h"
#include "util.h"

#define HOOK_VECTOR_INIT_AND_CHECK(context, type)												\
	do {																						\
		if (vector_new(context, &context->on_ ##type ##_vec) < 0) {								\
			_ERROR("%s: out of memory allocating hook vector for %s.", __FUNCTION__, "#type");	\
			ret = -1;																			\
			goto out;																			\
		}																						\
	} while (0)

static int hook_context_vectors_init(struct hook_context *context)
{
	int ret = -1;

	HOOK_VECTOR_INIT_AND_CHECK(context, player_join);
	HOOK_VECTOR_INIT_AND_CHECK(context, player_leave);

	ret = 0;
out:
	return ret;
}

int hook_context_new(TALLOC_CTX *context, const struct game *game, struct hook_context **out_context)
{
	int ret = -1;
	TALLOC_CTX *temp_context;

	struct hook_context *hook_context;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for hook context.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	hook_context = talloc(temp_context, struct hook_context);
	if (hook_context == NULL) {
		_ERROR("%s: out of memory allocating hook context.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	hook_context->game = (struct game *)game;
	
	if (hook_context_vectors_init(hook_context) < 0) {
		_ERROR("%s: error initializing hook subsystem.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	*out_context = talloc_steal(context, hook_context);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

void hook_on_player_leave(struct hook_context *hook_context, const struct game *game, const struct player *player)
{
	size_t i = 0;
	int cb_ret = 0;

	for (i = 0; i < hook_context->on_player_leave_vec->size; i++) {
		player_join_cb_t cb = vector_get(hook_context->on_player_leave_vec, i);

		if (cb == NULL) {
			continue;
		}

		if ((cb_ret = cb(game, player)) < 0) {
			_ERROR("%s: on_player_leave hook returned %d.\n", __FUNCTION__, cb_ret);
			return;
		}
	}
}



void hook_on_player_join(struct hook_context *hook_context, const struct game *game, const struct player *player)
{
	size_t i = 0;
	int cb_ret = 0;

	for (i = 0; i < hook_context->on_player_join_vec->size; i++) {
		player_join_cb_t cb = vector_get(hook_context->on_player_join_vec, i);

		if (cb == NULL) {
			continue;
		}

		if ((cb_ret = cb(game, player)) < 0) {
			_ERROR("%s: on_player_join hook returned %d.\n", __FUNCTION__, cb_ret);
			return;
		}
	}
}
