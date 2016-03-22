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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "talloc/talloc.h"
#include "binary_writer.h"
#include "util.h"

static int __7_bit_len(int value)
{
	int count = 0;

	uint32_t v = (uint32_t) value;   // support negative numbers
	while (v >= 0x80) {
		count++;
		v >>= 7;
	}

	return count;
}

void binary_writer_write_7bit_int(char *buf, int value, int *pos)
{
	uint32_t v = (uint32_t) value;   // support negative numbers

	while (v >= 0x80) {
		buf[*pos] = (uint8_t)(v | 0x80);
		v >>= 7;
		*pos++;
	}

	buf[*pos] = (uint8_t)v;
}

int binary_writer_write_string(TALLOC_CTX *context, const char *src, char **out_string)
{
	int len, pos = 0;
	char *string;

	len = strlen(src);

	/*
	 * 7-bit encoded length goes at the start of the string.
	 * Count how many bytes are in the 7-bit length to make
	 * room for it.
	 */
	string = talloc_size(context, len + __7_bit_len(len));
	if (string == NULL) {
		_ERROR("%s: out of memory allocating string for binary write.\n", __FUNCTION__);
		return -ENOMEM;
	}

	binary_writer_write_7bit_int(string, len, &pos);
	memcpy(string + pos, src, len); 

	*out_string = string;

	return 0;
}