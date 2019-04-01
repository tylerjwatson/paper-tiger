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

#define PACKET_TYPE_INVENTORY_SLOT 5
#define PACKET_LEN_INVENTORY_SLOT 7

#include <uv.h>

#include "../item.h"
#include "../talloc/talloc.h"

struct player;
struct packet;
struct game;

#ifdef __cplusplus
extern "C" {
#endif

int inventory_slot_new(TALLOC_CTX *ctx, const struct player *player, const struct item_slot slot, struct packet **out_packet);

int inventory_slot_read(struct packet *packet);

int inventory_slot_handle(struct player *player, struct packet *packet);


#ifdef __cplusplus
}
#endif