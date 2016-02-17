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

#include "binary_reader.h"

static int __increment_pos(struct binary_reader_context *context, size_t pos)
{
	int ret;

	if ((ret = fseek(context->fp, pos, SEEK_CUR)) < 0) {
		goto done;
	}

	context->file_pos += pos;

done:
	return ret;
}

int binary_reader_new(TALLOC_CTX *parent_context, const char *file_path, 
					  struct binary_reader_context **out_context)
{
	int ret = 0;
	TALLOC_CTX *tempContext;
	struct binary_reader_context *newContext;

	if ((tempContext = talloc_new(NULL)) == NULL) {
		ret = -ENOMEM;
		goto failed;
	}

	newContext = talloc_zero(tempContext, struct binary_reader_context);

	if ((newContext->file_path = talloc_strdup(newContext, file_path)) == NULL) {
		ret = -ENOMEM;
		goto failed;
	}

	newContext->file_pos = 0;
	*out_context = talloc_steal(parent_context, newContext);

failed:
	talloc_free(tempContext);
	return ret;
}

int binary_reader_open(struct binary_reader_context *context)
{
	if (context->fp != NULL) {
		return -1;
	}

	context->fp = fopen(context->file_path, "r");
	rewind(context->fp);
	context->file_pos = 0;

	return 0;
}

int binary_reader_read_boolean(struct binary_reader_context *context, bool *out_value);

int binary_reader_read_decimal(struct binary_reader_context *context, long double *out_value);

int binary_reader_read_double(struct binary_reader_context *context, double *out_value);

int binary_reader_read_int16(struct binary_reader_context *context, int16_t *out_value);

int binary_reader_read_int32(struct binary_reader_context *context, int32_t *out_value);

int binary_reader_read_int64(struct binary_reader_context *context, int64_t *out_value);

int binary_reader_read_single(struct binary_reader_context *context, float *out_value);

int binary_reader_read_string(struct binary_reader_context *context, char **out_value);

int binary_reader_read_uint16(struct binary_reader_context *context, uint16_t *out_value);

int binary_reader_read_uint32(struct binary_reader_context *context, uint32_t *out_value);

int binary_reader_read_uint64(struct binary_reader_context *context, uint64_t *out_value);

int binary_reader_close(struct binary_reader_context *context);