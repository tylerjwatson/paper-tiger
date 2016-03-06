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
 *
 * upgraded-guacamole is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with upgraded-guacamole.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _HAVE_PACKET_H
#define _HAVE_PACKET_H

#include <uv.h>
#include <stdint.h>

#include "player.h"
#include "packet_type.h"

struct packet_buffer {
    uint16_t len;
    enum packet_type type;

    char *data;
};

typedef int (*packet_handler_cb) (const struct player * player, const struct packet_buffer * buf);

struct packet_handler {
    uint8_t type;
    packet_handler_cb handler;
};

int packet_read_header(const uv_buf_t * buf, uint8_t * out_type, uint16_t * out_len);

#endif				/* _HAVE_PACKET_H */
