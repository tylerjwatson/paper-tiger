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

#include "connect_request.h"
#include "../binary_reader.h"
#include "../util.h"

int connect_request_new(struct packet *packet, const uv_buf_t *buf)
{
	TALLOC_CTX *temp_context;
	int ret = -1, pos = 0, str_len;
	struct connect_request *connect_request;
	
	temp_context = talloc_new(NULL);
	if (temp_context = NULL) {
		_ERROR("%s: out of memory allocating temp context for connect request.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	connect_request = talloc_zero(temp_context, struct connect_request);
	if (connect_request == NULL) {
		_ERROR("%s: out of memory allocating connect request.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	binary_reader_read_7bit_int(buf->base, &pos, &str_len);

	(buf->base + pos)[str_len] = '\0';

	connect_request->protocol_version = talloc_strdup(connect_request, buf->base + pos);
	connect_request->packet = packet;

	packet->data = (void *)talloc_steal(packet, connect_request);
	
	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}