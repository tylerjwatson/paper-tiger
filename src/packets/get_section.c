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

#include <string.h>
#include <stdbool.h> 

#include "get_section.h"

#include "../world.h"
#include "../game.h"
#include "../packet.h"
#include "../player.h"

#include "../talloc/talloc.h"
#include "../binary_reader.h"
#include "../binary_writer.h"
#include "../util.h"

#define ARRAY_SIZEOF(a) sizeof(a)/sizeof(a[0])

int get_section_handle(struct player *player, struct packet *packet)
{
	struct get_section *get_section = (struct get_section *)packet->data;

	if (get_section->x == -1 && get_section->y == -1) {
		//send spawn section
		
	}
	
	return 0;
}

int get_section_read(struct packet *packet, const uv_buf_t *buf)
{
	TALLOC_CTX *temp_context;
	int ret = -1, pos = 0;
	struct get_section *get_section;
	uint8_t *u_buffer = (uint8_t *)buf->base;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for get section.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	get_section = talloc_zero(temp_context, struct get_section);
	if (get_section == NULL) {
		_ERROR("%s: out of memory allocating get section.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	get_section->x = *(int32_t *)(u_buffer + pos);
	pos += sizeof(int32_t);
	get_section->y = *(int32_t *)(u_buffer + pos);

	packet->data = (void *)talloc_steal(packet, get_section);

	ret = 0;
out:
	talloc_free(temp_context);
	
	return ret;
}