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
#include "console.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "param.h"
#include "player.h"
#include "tile.h"
#include "util.h"
#include "world.h"

#include "packets/disconnect.h"

static int
__handle_memdump(struct game *game, struct console_command *command)
{
	FILE *fp;
	char file_path[48];
	time_t timer;
	struct tm *time_info;
	size_t len;

	time(&timer);
	time_info = localtime(&timer);

	len = strftime(file_path, sizeof(file_path), "memrpt-%y%m%d-%H.%M.%s.txt", time_info);
	file_path[len] = '\0';

	fp = fopen(file_path, "w+");
	if (fp == NULL) {
		_ERROR("%s: fopen for path %s failed.\n", __FUNCTION__, file_path);
		return -1;
	}

	talloc_report_full(game, fp);

	fclose(fp);

	_ERROR("%s: wrote %s.\n", __FUNCTION__, file_path);

	return 0;
}

static int
__handle_test(struct game *game, struct console_command *command)
{
	struct param *params;

	param_new(game, command->parameters, &params);

	printf("%s: %d params\n", command->command_name, params->num_parameters);

	for (int i = 0; i < params->num_parameters; i++) {
		printf("%s: match %d: %s\n", __FUNCTION__, i, params->parameters[i]);
	}

	param_free(params);

	return 0;
}

static int
__handle_quit(struct game *game, struct console_command *command)
{
	uv_stop(&game->event_loop);
	return 0;
}

static int
__handle_disconnect(struct game *game, struct console_command *command)
{
	int slot;
	struct player *player;
	struct packet *disconnect;

	if (command->parameters == NULL || strlen(command->parameters) == 0) {
		_ERROR("%s: slot required.  syntax: /disconnect {slot}\n", command->command_name);
		return 0;
	}

	slot = atoi(command->parameters);
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

static struct console_command_handler __command_handlers[] = {
	{.command_name = "test", .handler = __handle_test},
	{.command_name = "quit", .handler = __handle_quit},
	{.command_name = "disconnect", .handler = __handle_disconnect},
	{.command_name = "dc", .handler = __handle_disconnect},
	{.command_name = "memrpt", .handler = __handle_memdump},
	{0, 0}};

static void
__on_write(uv_write_t *req, int status)
{
	if (status < 0) {
		return;
	}

	talloc_free(req);
}

void
console_vsprintf(const struct console *console, const char *fmt, ...)
{
	uv_write_t *req = talloc(console, uv_write_t);
	char *line_buf = talloc_size(req, 1024);
	int len = 0;

	uv_buf_t buf = uv_buf_init(line_buf, 1024);
	va_list args;

	va_start(args, fmt);
	len = vsnprintf(buf.base, 1024, fmt, args);
	va_end(args);

	buf.len = len;

	uv_write(req, (uv_stream_t *)&console->console_write_handle, &buf, 1, __on_write);
}

static void
__print_prompt(uv_stream_t *stream)
{
	uv_write_t *req = talloc(stream->data, uv_write_t);
	static uv_buf_t buf = {
		.base = "[server@\033[32;1mpaper-tiger\033[0m] > ",
	};

	buf.len = strlen(buf.base);

	if (req == NULL) {
		_ERROR("%s: write request for console handle failed.\n", __FUNCTION__);
		return;
	}

	uv_write(req, stream, &buf, 1, __on_write);
}

static void
__on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
	char *command_copy, *command_copy_base;
	char *parameters;
	struct game *game = (struct game *)stream->data;
	struct console_command_handler *handler;
	struct console_command command;

	if (nread <= 0) {
		goto out;
	}

	command_copy_base = (char *)malloc(nread + 1);
	if (command_copy_base == NULL) {
		goto out;
	}

	memcpy(command_copy_base, buf->base, nread);
	command_copy_base[nread] = '\0';

	/*
	 * We are not interested in the preceding slash, skip the pointer
	 * by one to tar over the fact that it even exists.
	 */
	if (command_copy_base[0] == '/') {
		command_copy = command_copy_base + 1;
	} else {
		command_copy = command_copy_base;
	}

	/*
	 * Linefeed characters must be terminated from the input.
	 */
	command_copy = strtok(command_copy, " ");
	command_copy[strcspn(command_copy, "\r\n")] = '\0';
	parameters = buf->base + strcspn(buf->base, " ");

	if (strlen(command_copy) == 0) {
		goto out;
	}

	for (handler = __command_handlers; handler->command_name != NULL; handler++) {
		if (strcmp(handler->command_name, command_copy) != 0) {
			continue;
		}

		command.command_name = command_copy;
		command.parameters = parameters;

		if (handler->handler(game, &command) != 0) {
			_ERROR("%s: Error executing handler for %s command.\n", __FUNCTION__, command_copy);
			goto command_copy_free;
		}

		goto command_copy_free;
	}

	_ERROR("%s: unknown command.\n", command_copy);

command_copy_free:
	free(command_copy_base);
out:
	if (buf->base != NULL) {
		talloc_free(buf->base);
	}

	__print_prompt((uv_stream_t *)&game->console.console_write_handle);
}

static void
__alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	TALLOC_CTX *temp_context = talloc_new(NULL);
	struct game *context = (struct game *)handle->data;
	char *buffer;

	if ((buffer = talloc_zero_size(temp_context, suggested_size)) == NULL) {
		_ERROR("%s: Could not allocate %ld bytes for console input buffer.", __FUNCTION__, suggested_size);
		goto out;
	}

	*buf = uv_buf_init(talloc_steal(context, buffer), suggested_size);
out:
	talloc_free(temp_context);
}

int
console_init(struct console *console, struct game *game)
{
	console->game = game;
	uv_tty_init(&console->game->event_loop, &console->console_handle, 0, 1);
	uv_tty_init(&console->game->event_loop, &console->console_write_handle, 1, 0);

	uv_tty_set_mode(&console->console_handle, UV_TTY_MODE_NORMAL);

	console->console_handle.data = console->game;
	console->console_write_handle.data = console->game;

	uv_read_start((uv_stream_t *)&console->console_handle, __alloc_buffer, __on_read);

	__print_prompt((uv_stream_t *)&console->console_write_handle);

	return 0;
}
