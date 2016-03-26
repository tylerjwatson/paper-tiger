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
#ifndef UPGRADED_GUACAMOLE_UTIL_H
#define UPGRADED_GUACAMOLE_UTIL_H

#define _ERROR(x,...) fprintf(stderr, x,##__VA_ARGS__)

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define bit_toggle(src, n, val)	do {		\
	src ^= (-(int)val ^ src) & (1 << n);		\
} while (0)

#ifdef __cplusplus
}
#endif

#endif //UPGRADED_GUACAMOLE_UTIL_H
