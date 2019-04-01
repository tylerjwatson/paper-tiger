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

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rect {
	int32_t x;
	int32_t y;
	int16_t w;
	int16_t h;
};

static inline struct rect
rect_new(int x, int y, int w, int h)
{
	struct rect r;

	r.h = h;
	r.w = w;
	r.x = x;
	r.y = y;

	return r;
}

#ifdef __cplusplus
}
#endif
