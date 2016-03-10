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

static void __on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{

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

	*buf = uv_buf_init(talloc_steal(handle, buffer), suggested_size);
out:
	talloc_free(temp_context);
}

int console_new(TALLOC_CTX *context, uv_tty_t **out_tty_handle)
{
	int ret = -1;
	TALLOC_CTX *temp_context = talloc_new(NULL);
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