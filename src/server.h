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

#ifndef _HAVE_SERVER_H
#define _HAVE_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <uv.h>

#include "talloc/talloc.h"
#include "game.h"

struct server {
    char *listen_address;
    uint32_t port;		
    struct game *game;                      /* Backpointer to the game who owns it */
    
    uv_tcp_t *tcp_handle;
};

int server_new(TALLOC_CTX *context, const char *listen_address, const uint16_t port,
               struct game *game, struct server **out_server);

int server_start(struct server *server);

#ifdef __cplusplus
}
#endif

#endif
