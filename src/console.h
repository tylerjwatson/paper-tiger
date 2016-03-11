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

#ifndef _HAVE_CONSOLE_H
#define _HAVE_CONSOLE_H

#include <uv.h>

#include "game.h"

typedef int(*console_command_cb)(struct game_context *game, struct console_command *command);

struct console {
	uv_tty_t *console_handle;
	struct game_context *game;
};

struct console_command {
	char *command_name;
	char *getopt_options;
	char *parameters;
};

struct console_command_handler {
	char *command_name;
	char *help_text;
	console_command_cb handler;
};

int console_init(struct game_context *context);

#endif