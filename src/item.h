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

#ifndef _HAVE_ITEM_H
#define _HAVE_ITEM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct item_slot {
	uint8_t id;
	uint8_t slot_id;
	int16_t stack;
	uint8_t prefix;
	int16_t net_id;
};

#ifdef __cplusplus
}
#endif
	
#endif