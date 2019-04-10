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

#pragma once

#include <uv.h>

#include "game.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup console Console subsystem
 * @ingroup paper-tiger
 *
 * The console and command subsystem uses libuv tty handles to read input
 * from stdin (or another source) in a cross-platform manner, and pass the
 * string input to the command system once the parameters have been parsed.
 *
 */


int
ptConsoleInitialize(ptGame* game);

/**
 */

#ifdef __cplusplus
}
#endif
