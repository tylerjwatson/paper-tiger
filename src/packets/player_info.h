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

#ifndef _HAVE_PLAYER_INFO_H
#define _HAVE_PLAYER_INFO_H

#define PACKET_TYPE_PLAYER_INFO 0x03

/*
 * Base length + name length + 1
 */
#define PACKET_LEN_PLAYER_INFO 29

#include <uv.h>
#include "../packet.h"
#include "../colour.h"

#ifdef __cplusplus
extern "C" {
#endif

struct player_info {
	uint8_t id;
	uint8_t skin_variant;
	uint8_t hair;
	char *name;
	uint8_t hair_dye;
	uint8_t hide_visuals;
	uint8_t hide_visuals2;
	uint8_t hide_misc;

	struct colour hair_colour;
	struct colour skin_colour;
	struct colour eye_colour;
	struct colour shirt_colour;
	struct colour under_shirt_colour;
	struct colour pants_colour;
	struct colour shoe_colour;

	uint8_t difficulty;
};

int player_info_new(TALLOC_CTX *ctx, const struct player *player, struct packet **out_packet);

int player_info_write(TALLOC_CTX *context, struct packet *packet, uv_buf_t *buf);

int player_info_read(struct packet *packet, const uv_buf_t *buf);


#ifdef __cplusplus
}
#endif

#endif //_HAVE_player_info_H