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

#include <uv.h>

#include "game.h"
#include "util.h"
#include "server.h"
#include "player.h"

static void __on_read(uv_stream_t * stream, ssize_t len, const uv_buf_t * buf)
{
    struct player *player = (struct player *) stream->data;

    if (len < 0) {
	if (len == UV_EOF) {
	    _ERROR("%s: EOF while reading from slot %d\n", __FUNCTION__, player->id);
	}

	goto player_out;
    }

    _ERROR("%s: Read %ld bytes from client at slot %d\n", __FUNCTION__, len, player->id);

    goto out;

  player_out:
    _ERROR("%s: read error from slot %d", __FUNCTION__, player->id);
    player_close(player);

  out:
    talloc_free(buf->base);
}

static void __alloc_buffer(uv_handle_t * handle, size_t size, uv_buf_t * out_buf)
{
    struct player *player = (struct player *) handle->data;

    if (player->state == PLAYER_SOCKET_STATE_HEADER) {
	size = 2;
    }

    *out_buf = uv_buf_init(talloc_size(player, size), size);
}

void __on_connection(uv_stream_t * handle, int status)
{
    struct sockaddr_in peer;
    struct server *server = (struct server *) handle->data;
    struct player *player;
    int name_len = sizeof(peer);
    int player_id;
    char remote_addr[16] = { };

    uv_tcp_t *client_handle;

    if (status < 0) {
	return;
    }

    /*
     * Re-listen for more clients
     */


    if ((player_id = game_find_next_slot(server->game)) < 0) {
	return;
    }

    if (player_new(server->game, server->game, player_id, &player) < 0) {
	_ERROR("%s: Allocating a player object for ID %d failed.", __FUNCTION__, player_id);
	uv_close((uv_handle_t *) & client_handle, NULL);
	return;
    }

    client_handle = talloc(player, uv_tcp_t);

    uv_tcp_init(server->game->event_loop, client_handle);
    uv_tcp_nodelay(client_handle, true);

    if (uv_accept(handle, (uv_stream_t *) client_handle) < 0) {
	_ERROR("%s: Could not accept socket.", __FUNCTION__);
	return;
    }

    player->handle = client_handle;
    player->handle->data = player;

    uv_tcp_getpeername((uv_tcp_t *) & client_handle, (struct sockaddr *) &peer, &name_len);
    uv_inet_ntop(AF_INET, &peer.sin_addr, remote_addr, sizeof(remote_addr));

    player->remote_addr = talloc_strdup(player, remote_addr);
    player->remote_port = peer.sin_port;

    _ERROR("%s: %s has connected to slot %d\n", __FUNCTION__, remote_addr, player_id);

    // start read
    server->game->players[player_id] = player;
    uv_read_start((uv_stream_t *) player->handle, __alloc_buffer, __on_read);
}

int server_new(TALLOC_CTX * context, const char *listen_address,
	       const uint16_t port, struct game_context *game, struct server **out_server)
{
    int ret = -1;
    TALLOC_CTX *temp_context;
    struct server *server;

    if ((temp_context = talloc_new(NULL)) == NULL || (server = talloc_zero(temp_context, struct server)) == NULL) {
	_ERROR("%s: Cannot allocate a server object.\n", __FUNCTION__);
	ret = -ENOMEM;
	goto out;
    }

    server->listen_address = talloc_strdup(server, listen_address);
    server->port = port;
    server->game = game;

    *out_server = talloc_steal(context, server);

    ret = 0;
  out:
    talloc_free(temp_context);

    return ret;
}

int server_start(struct server *server)
{
    struct sockaddr_in server_addr;

    uv_tcp_init(server->game->event_loop, &server->tcp);
    uv_ip4_addr(server->listen_address, server->port, &server_addr);
    uv_tcp_bind(&server->tcp, (const struct sockaddr *) &server_addr, 0);

    server->tcp.data = server;

    if (uv_listen((uv_stream_t *) & server->tcp, 128, __on_connection) != 0) {
	_ERROR("%s: listen failure.\n", __FUNCTION__);
	return -1;
    }

    return 0;
}
