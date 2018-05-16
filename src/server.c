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
#include "packet.h"
#include "player.h"
#include "util.h"

static int
__handle_packet(struct player *player, struct packet *packet)
{
	struct packet_handler *handler;
	int ret = -1;

	if (packet_deserialize(packet) < 0) {
		_ERROR("%s: packet deserialize failed for type %d in slot %d.\n", __FUNCTION__, packet->type, player->id);
		return -1;
	}

	handler = packet_handler_for_type(packet->type);
	if (handler == NULL || handler->handle_func == NULL) {
		_ERROR("%s: unknown message of type %d for slot %d.\n", __FUNCTION__, packet->type, player->id);
		return -1;
	}

	ret = handler->handle_func(player, packet);
	if (ret < 0) {
		_ERROR("%s:handler failed for type %d from slot %d.\n", __FUNCTION__, packet->type, player->id);
		return -1;
	}

	return 0;
}

static void
__on_read(uv_stream_t *stream, ssize_t len, const uv_buf_t *buf)
{
	struct player *player = (struct player *)stream->data;
	struct packet *incoming_packet = (struct packet *)player->incoming_packet;

	if (len < 0) {
		player_close(player);
		return;
	}

	if (len == 0) {
		return;
	}

	if (incoming_packet->state == packet_state_read_data) {
		if (__handle_packet(player, incoming_packet) < 0) {
			_ERROR("%s: packet handler for type %d failed.\n", __FUNCTION__, incoming_packet->type);
		}

		talloc_free(incoming_packet);
		player->incoming_packet = NULL;
	}
}

static void
__alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *out_buf)
{
	struct player *player = (struct player *)handle->data;
	struct packet *incoming_packet = player->incoming_packet;
	uint8_t *buffer;
	int16_t len;

	if (incoming_packet == NULL) {
		incoming_packet = talloc(player, struct packet);
		if (incoming_packet == NULL) {
			_ERROR("%s: out of memory allocating incoming packet buffer\n", __FUNCTION__);
			return;
		}

		packet_init(incoming_packet);
		player->incoming_packet = incoming_packet;

		len = sizeof(player->incoming_packet->header);
		buffer = player->incoming_packet->header;

		incoming_packet->state = packet_state_read_header;
	} else {
		len = incoming_packet->len - PACKET_HEADER_SIZE;
		buffer = incoming_packet->data_buffer;

		incoming_packet->state = packet_state_read_data;
	}

	*out_buf = uv_buf_init((char *)buffer, len);
}

void
__on_connection(uv_stream_t *handle, int status)
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

	uv_tcp_init(&server->game->event_loop, client_handle);
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

static void
__on_write(uv_write_t *req, int status)
{
	talloc_free(req);
}

int
server_send(const struct server *server, struct packet *packet)
{
	int ret = 0;

	/*
	 * Re-parent the packet underneath the server, the server owns it now
	 * and is reponsible for keeping it alive until the packet is sent to
	 * all recipients.
	 */
	talloc_steal(server, packet);

	return ret;
}

int
server_send_packet(const struct server *server, const struct player *player, const struct packet *packet)
{
	return -1;
}

int
server_init(TALLOC_CTX *context, struct server *server, const char *listen_address, const uint16_t port)
{
	server->port = port > 0 ? port : 7777;
	server->listen_address = talloc_strdup(context, listen_address);

	return 0;
}

int
server_start(struct server *server)
{
	struct sockaddr_in server_addr;

	uv_tcp_init(&server->game->event_loop, &server->tcp_handle);
	uv_ip4_addr(server->listen_address, server->port, &server_addr);
	uv_tcp_bind(&server->tcp_handle, (const struct sockaddr *)&server_addr, 0);

	server->tcp_handle.data = server;

	if (uv_listen((uv_stream_t *)&server->tcp_handle, 128, __on_connection) != 0) {
		_ERROR("%s: listen failure.\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

int
server_broadcast_packet(const struct server *server, const struct packet *packet, int8_t ignore_id)
{
	return 0;
}
