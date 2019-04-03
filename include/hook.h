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

#pragma once

#include "talloc/talloc.h"
#include "vector.h"
#include "game.h"

#define HOOK_DECL(type, cb)                                                                                            \
	static inline void hook_##type##_register(struct hook_context *context, cb callback)                               \
	{                                                                                                                  \
		vector_push_back(context->on_##type##_vec, (void *)callback);                                                  \
	}                                                                                                                  \
                                                                                                                       \
	static inline void hook_##type##_deregister(struct hook_context *context, cb callback)                             \
	{                                                                                                                  \
		vector_delete(context->on_##type##_vec, (void *)callback);                                                     \
	}

#ifdef __cplusplus
extern "C" {
#endif

struct vector;
struct player;

typedef int (*player_join_cb_t)(const ptGame *game, const struct player *player);
typedef int (*player_leave_cb_t)(const ptGame *game, const struct player *player);

struct hook_context {
	ptGame *game;

	struct vector *on_player_join_vec;
	struct vector *on_player_leave_vec;
};

int
hook_context_new(TALLOC_CTX *context, const ptGame *game, struct hook_context **out_context);

HOOK_DECL(player_join, player_join_cb_t)

void
hook_on_player_join(struct hook_context *hook_context, const ptGame *game, const struct player *player);

HOOK_DECL(player_leave, player_leave_cb_t)

void
hook_on_player_leave(struct hook_context *hook_context, const ptGame *game, const struct player *player);

#ifdef __cplusplus
}
#endif
