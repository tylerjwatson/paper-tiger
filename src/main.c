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

#include <talloc.h>
#include <errno.h>

#include "game.h"
#include "binary_reader.h"

int main(int argc, char **argv)
{
	int ret = 0;
	struct game_context *gameContext;

	printf("Upgraded Guacamole\n");

	struct binary_reader_context *br;
	
	if ((ret = binary_reader_new(NULL, "PTS_150829.wld", &br)) < 0) {
		printf("binary reader init failed: %d\n", ret);
	}
	
	if (game_new(NULL, &gameContext) < 0) {
		printf("game init failed.\n");
		return -1;
	}

	talloc_free(br);
	talloc_free(gameContext);
}

