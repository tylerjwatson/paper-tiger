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

#ifndef _HAVE_CONSOLE_H
#define _HAVE_CONSOLE_H

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
 * @{
 */

/**
 * Describes a console context in which all memory for the console subsystem is
 * allocated under.
 */
struct console {
	/**
	 * A reference to the libuv tty handle which reads from and to the owning
	 * terminal.
	 */
	uv_tty_t *console_handle;

	/**
	 * Backpointer to the game which owns the instance of the console context.
	 */
	struct game *game;
};

/**
 * Describes a console command.
 */
struct console_command {
	/**
	 * Contains the command name without the prefixed '/' command specifier.
	 */
	char *command_name;

	/**
	 * Contains the parameter string which hasn't been split yet.
	 */
	char *parameters;
};

/**
 * Describes a function to be called when a command is entered into the console.
 */
typedef int(*console_command_cb)(struct game *game, struct console_command *command);


/** 
 * Describes a console command handler which is entered into the table of command
 * handlers in @a console.c.
 */
struct console_command_handler {
	char *command_name;
	char *help_text;
	console_command_cb handler;
};

/**
 * @brief	Allocate a new console context.
 * 
 * Allocates a new console structure underneath the talloc context pointed to by @a context.
 * 
 * @param[in]		context		A pointer to the talloc context in which to allocate this memory
 * 								under
 * @param[in]		game		A pointer to the game context which owns this console context
 * @param[out]		out_console	A pointer to the address where the allocated console object will
 * 								be stored.
 * 
 * @remarks
 * On successful return of this function, the console handle may be initialized by calling
 * @a console_init.
 * 
 * @returns
 * `0` if the allocation of a new console context succeeded, or `< 0` otherwise.
 */
int console_new(TALLOC_CTX *context, struct game *game, struct console **out_console);

/**
 * @brief Initializes a newly-allocated console context.
 * 
 * Initializes the libuv tty handle contained in @ref console->console_handle to accept input
 * to and from stdin.
 * 
 * @param[in]		console		A pointer to the console object allocated with a call to
 * 								@a console_new
 * 
 * @returns
 * `0` if the initialization was successful, `< 0` otherwise.  If this function cannot
 * return successfully, paper tiger will be unable to process console input.
 */
int console_init(struct console *console);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
