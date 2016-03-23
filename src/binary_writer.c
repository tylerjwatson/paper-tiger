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

int binary_writer_7bit_len(int value)
{
	int count = 1;

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
		(*pos)++;
	}

	buf[*pos] = (uint8_t)v;
	(*pos)++;
}

int binary_writer_write_string(char *dest, const char *src)
{
	int len, pos = 0;
	
	len = strlen(src);

	/*
	 * 7-bit encoded length goes at the start of the string.
	 */

	binary_writer_write_7bit_int(dest, len, &pos);
	memcpy(dest + pos, src, len); 

	return len + binary_writer_7bit_len(len);
}