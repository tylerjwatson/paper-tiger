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
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "command.h"
#include "param.h"
#include "game.h"
#include "player.h"
#include "server.h"
#include "util.h"

#include "packets/disconnect.h"

static int __handle_test(struct game *game, struct command *command)
{
	for (int i = 0; i < command->parameters->num_parameters; i++) {
		printf("%s: match %d: %s\n", __FUNCTION__, i, command->parameters->parameters[i]);
	}

	return 0;
}

static int __handle_quit(struct game *game, struct command *command)
{
	uv_stop(game->event_loop);
	return 0;
}

static int __handle_disconnect(struct game *game, struct command *command)
{
	int slot;
	struct player *player;
	struct packet *disconnect;

	if (command->parameters == NULL || command->parameters->num_parameters == 0) {
		_ERROR("%s: slot required.  syntax: /disconnect {slot}\n", command->command_name);
		return 0;
	}

	slot = atoi(command->parameters->parameters[1]);
	player = game->players[slot];

	if (player == NULL) {
		_ERROR("%s: there is no player at slot %d.\n", command->command_name, slot);
		return 0;
	}

	if (disconnect_new(player, player, "Test", &disconnect) < 0) {
		_ERROR("%s: out of memory sending packet.\n", __FUNCTION__);
		return -2;
	}

	server_send_packet(player->game->server, player, disconnect);
	
	talloc_free(disconnect);

	player_close(player);

	return 0;
}

int command_new(struct game *game, const char *command_name, int num_callbacks, command_cb_t *callbacks)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct command *command;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temporary context for command structure.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	command = talloc(temp_context, struct command);
	if (command == NULL) {

	}

	command->flags |= COMMAND_FLAGS_ENABLED;

	ret = 0;
out:
	talloc_free(temp_context);
	return ret;
}

int command_handle_console(const struct game *game, const char *line);
int command_handle_player(const struct player *player, const char *line);

struct command *command_find(struct game *game, const char *command_name);

int command_add_handler(struct command *command, command_cb_t callback);
int command_remove_handler(struct command *command, command_cb_t callback);

static int command_array_realloc(struct game *game, size_t new_size)
{
	int ret = -1;

	game->commands = talloc_realloc(game, game->commands, struct command *, new_size);
	if (game->commands == NULL) {
		_ERROR("%s: out of memory allocating enough space for command array.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	game->commands_size = new_size;

	ret = 0;
out:
	return ret;
}

int command_add(struct game *game, struct command *command)
{
	if (game->num_commands > game->commands_size) {
		if (command_array_realloc(game, game->commands_size << 1)) {
			_ERROR("%s: error reallocating command array.\n", __FUNCTION__);
			return -ENOMEM;
		}
	}

	game->commands[game->num_commands++] = talloc_steal(game->commands, command);

	return 0;
}
