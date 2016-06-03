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

#include "rect.h"
#include "vector_2d.h"
#include "world_section.h"
#include "world.h"
#include "util.h"

int world_section_init(struct world *world)
{
	int ret = -1;
	TALLOC_CTX *temp_context;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		ret = -ENOMEM;
		_ERROR("%s: out of memory allocating temp context.\n", __FUNCTION__);
		goto out;
	}

out:
	talloc_free(temp_context);

	return ret;
}

int world_section_to_coords(const struct world *world, unsigned section, struct vector_2d *out_coords)
{
	struct vector_2d coords;

	coords.x = section / world->max_sections_x;
	coords.y = section % world->max_sections_x;

	*out_coords = coords;

	return 0;
}

int world_section_to_tile_rect(const struct world *world, unsigned section, struct rect *out_rect)
{
	struct rect r;
	struct vector_2d section_coords;

	if (section > world->max_sections) {
		return -1;
	}

	world_section_to_coords(world, section, &section_coords);

	r.x = section_coords.x * WORLD_SECTION_WIDTH;
	r.y = section_coords.y * WORLD_SECTION_HEIGHT;

	*out_rect = r;

	return 0;
}