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

#ifndef _HAVE_CONTINUE_CONNECTING2_H
#define _HAVE_CONTINUE_CONNECTING2_H

#define PACKET_TYPE_CONTINUE_CONNECTING2 6
#define PACKET_LEN_CONTINUE_CONNECTING2 0

#include <uv.h>
#include "../talloc/talloc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct player;
struct packet;	

int continue_connecting2_handle(struct player *player, struct packet *packet);

#ifdef __cplusplus
}
#endif

#endif //_HAVE_CONTINUE_CONNECTING2_H