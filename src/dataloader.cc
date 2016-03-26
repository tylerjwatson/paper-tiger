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
 *
 * paper-tiger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paper-tiger.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <fstream>
#include <stdbool.h>

#include "talloc/talloc.h"

#include "dataloader.h"
#include "util.h"
#include "game.h"
#include "world.h"

#include "tile_flags.pb.h"

int dataloader_load_tile_flags(struct game *game)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	bool *tile_frame_important;
	tile_flags tile_flags;
	static const char *file_path = "data/tile/flags.dat";

	{
		std::fstream input_file(file_path, std::ios::in | std::ios::binary);

		if (tile_flags.ParseFromIstream(&input_file) == false) {
			_ERROR("%s: cannot parse data from %s.\n", __FUNCTION__, file_path);
			return -1;
		}
	}

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temp context for tile flags.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	tile_frame_important = talloc_zero_array(temp_context, bool, tile_flags.tile_frame_important_size());
	if (tile_frame_important == NULL) {
		_ERROR("%s: out of memory allocating tile frame important array.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	for (int i = 0; i < tile_flags.tile_frame_important_size(); i++) {
		const bool& flags = tile_flags.tile_frame_important(i);
		tile_frame_important[i] = flags;
	}

	game->tile_frame_important = talloc_steal(game, tile_frame_important);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}