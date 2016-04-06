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

#include "server.h"

#include <uv.h>

#include "game.h"
#include "util.h"
#include "player.h"
#include "packet.h"

static int __handle_packet(struct player *player, struct packet *packet, const uv_buf_t *buf)
{
	struct packet_handler *packet_handler;
	int ret = -1;

	packet_handler = packet_handler_for_type(packet->type);
	if (packet_handler == NULL) {
		_ERROR("error: received unknown message type %d from slot %d.\n", packet->type, player->id);
		return -1;
	}

	/*
	 * If the read handler for the function is NULL then we are safe
	 * to assume that the packet has no body.
	 */
	if (packet_handler->read_func != NULL && (ret = packet_handler->read_func(packet, buf)) < 0) {
		_ERROR("%s: packet parsing failed for slot %d.\n", __FUNCTION__, 
			player->id);
		return ret;
	}

	if (packet_handler->handle_func != NULL && (ret = packet_handler->handle_func(player, packet)) < 0) {
		_ERROR("%s: read handler for packet failed for slot %d.\n", __FUNCTION__, player->id);
		return ret;
	}

	return 0;
}

static void __on_read(uv_stream_t *stream, ssize_t len, const uv_buf_t *buf)
{
	struct player *player = (struct player *)stream->data;

	if (len < 0) {
		//_ERROR("%s: %s while reading from slot %d\n", __FUNCTION__, uv_err_name(len), player->id);
		player_close(player);
		return;
	}

	if (len == 0) {
		goto out;
	}

	if (player->incoming_packet == NULL && len == PACKET_HEADER_SIZE) {
		if (packet_new(player, player, buf, &player->incoming_packet) < 0) {
			_ERROR("%s: error parsing packet header for slot %d\n", __FUNCTION__, player->id);
			goto player_out;
		}

		if (player->incoming_packet->len == PACKET_HEADER_SIZE) {
			goto handle_packet;
		} 

		goto out;
	}

handle_packet:
	__handle_packet(player, player->incoming_packet, buf);

	talloc_free(player->incoming_packet);
	player->incoming_packet = NULL;

	goto out;

player_out:
	_ERROR("%s: read error from slot %d", __FUNCTION__, player->id);
	player_close(player);
	
out:
	talloc_free(buf->base);
}

static void __alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *out_buf)
{
	struct player *player = (struct player *)handle->data;

	if (player->incoming_packet == NULL) {
		size = PACKET_HEADER_SIZE;
	}
	else {
		size = player->incoming_packet->len - PACKET_HEADER_SIZE;
	}
	
	*out_buf = uv_buf_init(talloc_zero_size(player, size), size);
}

void __on_connection(uv_stream_t *handle, int status)
{
	struct sockaddr_in peer;
	struct server *server = (struct server *)handle->data;
	struct player *player;
	int name_len = sizeof(peer);
	int player_id;
	char remote_addr[16];
	
	uv_tcp_t *client_handle;
		
	if (status < 0) {
		return;
	}
	
	if ((player_id = game_find_next_slot(server->game)) < 0) {
		return;
	}
	
	if (player_new(server->game, server->game, player_id, &player) < 0) {
		_ERROR("%s: Allocating a player object for ID %d failed.", __FUNCTION__, player_id);
		uv_close((uv_handle_t *)&client_handle, NULL);
		return;
	}
	
	client_handle = talloc(player, uv_tcp_t);
	
	uv_tcp_init(server->game->event_loop, client_handle);
	uv_tcp_nodelay(client_handle, true);
	
	if (uv_accept(handle, (uv_stream_t *)client_handle) < 0) {
		_ERROR("%s: Could not accept socket.", __FUNCTION__);
		return;
	}
	
	player->handle = client_handle;
	player->handle->data = player;
	
	uv_tcp_getpeername((uv_tcp_t *)client_handle, (struct sockaddr *)&peer, &name_len);
	uv_inet_ntop(AF_INET, &peer.sin_addr, remote_addr, sizeof(remote_addr));
	
	player->remote_addr = talloc_strdup(player, remote_addr);
	player->remote_port = peer.sin_port;
	
	_ERROR("%s: %s has connected to slot %d\n", __FUNCTION__, remote_addr, player_id);
	
	// start read
	server->game->players[player_id] = player;
	uv_read_start((uv_stream_t *)player->handle, __alloc_buffer, __on_read);
}

static void __on_write(uv_write_t *req, int status)
{
	//talloc_free(req->write_buffer.base);  // WTF???? double-free?????
	talloc_free(req);
}

static int server_packet_to_buffer(const struct game *game, const uv_write_t *req, const struct packet *packet, uv_buf_t *bufs)
{
	struct packet_handler *packet_handler;
	char *header_buf, *payload_buf;

	int ret = 0, packet_len = 0, pos = 0;

	header_buf = talloc_zero_size(req, PACKET_HEADER_SIZE);
	if (header_buf == NULL) {
		_ERROR("%s: out of memory allocating packet header buffer.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	payload_buf = talloc_zero_size(req, PACKET_PAYLOAD_SIZE);
	if (payload_buf == NULL) {
		_ERROR("%s: out of memory allocating packet payload buffer.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	bufs[0] = uv_buf_init(header_buf, PACKET_HEADER_SIZE);
	bufs[1] = uv_buf_init(payload_buf, PACKET_PAYLOAD_SIZE);

	packet_handler = packet_handler_for_type(packet->type);
	if (packet_handler == NULL) {
		_ERROR("%s: could not find packet handler for type %d.\n", __FUNCTION__, packet->type);
		ret = -1;
		goto out;
	}

	if (packet_handler->write_func != NULL && (packet_len = packet_handler->write_func(game, packet, bufs[1])) < 0) {
		_ERROR("%s: error in write handler for packet type %d.\n", __FUNCTION__, packet->type);
		ret = -1;
		goto out;
	}

	if (packet_len >= 65535) {
		_ERROR("%s: packet overran max length.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	bufs[1].len = packet_len;

	packet_write_header(packet->type, packet_len + PACKET_HEADER_SIZE, &bufs[0], &pos);

out:
	return ret;
}

int server_send_packet(const struct server *server, const struct player *player, const struct packet *packet)
{
	TALLOC_CTX *temp_context;
	uv_write_t *write_request;
	uv_buf_t bufs[2];
	int pos = 0, ret = -1;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temporary context for packet buffers.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	write_request = talloc(temp_context, uv_write_t);
	if (write_request == NULL) {
		_ERROR("%s: out of memory writing packet to slot %d\n", __FUNCTION__, player->id);
		ret = -ENOMEM;
		goto out;
	}

	server_packet_to_buffer(server->game, (const uv_write_t *)write_request, packet, bufs);

	write_request->data = (void *)player;
	
	if (uv_write(talloc_steal(player->handle, write_request), (uv_stream_t *)player->handle, bufs, bufs[1].base != NULL ? 2 : 1, __on_write) < 0) {
		_ERROR("%s: write failed to slot %d.\n", __FUNCTION__, player->id);
		goto out;
	}

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}


int server_new(TALLOC_CTX *context, const char *listen_address, const uint16_t port,
			   struct game *game, struct server **out_server)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct server *server;
	uv_tcp_t *tcp_handle;
	
	if ((temp_context = talloc_new(NULL)) == NULL
		|| (server = talloc_zero(temp_context, struct server)) == NULL) {
		_ERROR("%s: Cannot allocate a server object.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	tcp_handle = talloc(temp_context, uv_tcp_t);
	if (tcp_handle == NULL) {
		_ERROR("%s: Cannot allocate a TCP handle.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}
	
	server->listen_address = talloc_strdup(server, listen_address);
	server->port = port;
	server->game = game;
	server->tcp_handle = talloc_steal(context, tcp_handle);

	*out_server = talloc_steal(context, server);
	
	ret = 0;
out:
	talloc_free(temp_context);
	
	return ret;
}

int server_start(struct server *server)
{
	struct sockaddr_in server_addr;
	
	uv_tcp_init(server->game->event_loop, server->tcp_handle);
	uv_ip4_addr(server->listen_address, server->port, &server_addr);
	uv_tcp_bind(server->tcp_handle, (const struct sockaddr *)&server_addr, 0);
	
	server->tcp_handle->data = server;
	
	if (uv_listen((uv_stream_t *)server->tcp_handle, 128, __on_connection) != 0) {
		_ERROR("%s: listen failure.\n", __FUNCTION__);
		return -1;
	}
	
	return 0;
}

int server_broadcast_packet(const struct server *server, const struct packet *packet)
{
	TALLOC_CTX *temp_context;
	uv_write_t *write_request;
	uv_buf_t bufs[2];
	uint8_t online_slots[GAME_MAX_PLAYERS];
	int online_len;

	int pos = 0, ret = -1;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temporary context for packet buffers.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	write_request = talloc(temp_context, uv_write_t);
	if (write_request == NULL) {
		_ERROR("%s: out of memory writing packet\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	server_packet_to_buffer(server->game, (const uv_write_t *)write_request, packet, bufs);

	online_len = game_online_players(server->game, online_slots);
	for (int i = 0; i < online_len; i++) {
		struct player *player = server->game->players[i];

		if (uv_write(talloc_steal(server, write_request), (uv_stream_t *)player->handle, bufs, bufs[1].base != NULL ? 2 : 1, __on_write) < 0) {
			_ERROR("%s: write failed to slot %d.\n", __FUNCTION__, player->id);
			goto out;
		}
	}

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}