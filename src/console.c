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

#include "console.h"
#include "game.h"
#include "util.h"

static int __handle_quit(struct game_context *game, struct console_command *command)
{
	uv_stop(game->event_loop);
	return 0;
}

static int __num_handlers = 1;
static struct console_command_handler __command_handlers[] = {
	{ .command_name = "quit", .handler = __handle_quit }
};

static void __on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
	char *command_copy;
	char *parameters;
	struct game_context *game = (struct game_context *)stream->data;
	struct console_command_handler *handler;
	struct console_command command;

	if (nread <= 0) {
		goto out;
	}

	command_copy = strdup(buf->base);
	if (command_copy == NULL) {
		goto out;
	}

	/*
	 * We are not interested in the preceding slash, skip the pointer
	 * by one to tar over the fact that it even exists.
	 */
	if (command_copy[0] == '/') {
		command_copy++;
	}

	/*
	 * Linefeed characters must be terminated from the input.
	 */
	command_copy = strtok(command_copy, " ");
	command_copy[strcspn(command_copy, "\r\n")] = '\0';
	parameters = buf->base + strcspn(buf->base, " ");
	
	for (int i = 0; i < __num_handlers; i++) {
		handler = &__command_handlers[i];

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

	_ERROR("console: error: %s: unknown command.\n", command_copy);

command_copy_free:
	free(command_copy);
out:
	talloc_free(buf->base);
}

static void __alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	TALLOC_CTX *temp_context = talloc_new(NULL);
	struct game_context *context = (struct game_context *)handle->data;
	char *buffer;

	if ((buffer = talloc_size(temp_context, suggested_size)) == NULL) {
		_ERROR("%s: Could not allocate %ld bytes for console input buffer.", __FUNCTION__, suggested_size);
		goto out;
	}

	*buf = uv_buf_init(talloc_steal(context, buffer), suggested_size);
out:
	talloc_free(temp_context);
}

int console_new(TALLOC_CTX *context, uv_tty_t **out_tty_handle)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	uv_tty_t *tty_handle;

	if ((temp_context = talloc_new(NULL)) == NULL) {
		_ERROR("%s: cannot allocate temporary context for console handle.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	if ((tty_handle = talloc_zero(temp_context, uv_tty_t)) == NULL) {
		_ERROR("%s: Cannot allocate console handle.\n", __FUNCTION__);
		ret = -ENOMEM;
	}

	ret = 0;
	*out_tty_handle = talloc_steal(context, tty_handle);
out:
	talloc_free(temp_context);

	return ret;
}

int console_init(struct game_context *context)
{
	uv_tty_t *tty_handle;

	if (console_new(context, &tty_handle) < 0) {
		_ERROR("%s: Console initialization failed.\n", __FUNCTION__);
		return -1;
	}

	uv_tty_init(context->event_loop, tty_handle, 0, true);
	uv_tty_set_mode(tty_handle, UV_TTY_MODE_NORMAL);

	tty_handle->data = context;
	context->console_handle = tty_handle;

	uv_read_start((uv_stream_t *)tty_handle, __alloc_buffer, __on_read);

	return 0;
error:
	talloc_free(tty_handle);
	return -1;
}