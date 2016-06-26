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

#ifdef __cplusplus
extern "C" {
#endif

struct command;		/* Forward decl */
struct game;
struct player;
struct param;

/**
 * Describes a function to be called when a command is entered into the console.
 */
typedef int(*command_cb_t)(struct game *game, const struct player *player, const struct command *command);

enum {
	COMMAND_FLAGS_ENABLED = 1,
};

/**
 * Describes a console command.
 */
struct command {
	/**
	 * Contains the command name without the prefixed '/' command specifier.
	 */
	char *command_name;

	/**
	 * A pointer to the structure containing all the parameters to the command
	 * typed from the console or from a player.
	 */
	struct param *parameters;

	/**
	 * Contains the number of handler callbacks attached to this comand.
	 */
	int num_handlers;

	/**
	 * Pointer to an array of function addresses to call when the command
	 * has been entered in the console.
	 */
	command_cb_t *handlers;

	/**
	 * Describes a set of flags for this command.  See @a COMMAND_FLAGS enumeration
	 */
	int flags;
};

int command_new(struct game *game, const char *command_name, int num_callbacks, command_cb_t *callbacks);

int command_handle_console(const struct game *game, const char *line);
int command_handle_player(const struct player *player, const char *line);

struct command *command_find(struct game *game, const char *command_name);

int command_add_handler(struct command *command, command_cb_t callback);
int command_remove_handler(struct command *command, command_cb_t callback);

int command_add(struct game *game, struct command *command);



/**
 * @}
 */

#ifdef __cplusplus
}
#endif

