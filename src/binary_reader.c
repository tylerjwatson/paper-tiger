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

#include "binary_reader.h"
#include "util.h"

static uint16_t le16_to_cpu(const uint8_t *buf)
{
	return ((uint16_t)buf[0]) | (((uint16_t)buf[1]) << 8);
}

static uint16_t be16_to_cpu(const uint8_t *buf)
{
	return ((uint16_t)buf[1]) | (((uint16_t)buf[0]) << 8);
}

static int __read_7_bit_int(struct binary_reader_context *context, int32_t *out_value) {
	int count = 0, shift = 0;
	uint8_t byte;

	do {
		if (shift == 5 * 7) {
			return -1;
		}

		if (binary_reader_read_byte(context, &byte) < 0) {
			return -1;
		}

		count |= (byte & 0x7F) << shift;
		shift += 7;
	} while ((byte & 0x80) != 0);

	*out_value = count;

	return 0;
}

static int __binary_reader_destructor(struct binary_reader_context *talloc_context)
{
	//struct binary_reader_context *context = talloc_get_type_abort(talloc_context, struct binary_reader_context);
	binary_reader_close(talloc_context);
	return 0;
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

	/*
	 * Destructor callback makes sure the file pointer is closed
	 * before the structure is deallocated with talloc_free.
	 */
	talloc_set_destructor(newContext, __binary_reader_destructor);

	if (out_context != NULL) {
		*out_context = talloc_steal(parent_context, newContext);
	}
	
failed:
	talloc_free(tempContext);
	return ret;
}

size_t binary_reader_pos(struct binary_reader_context *context)
{
	return ftell(context->fp);
}

int binary_reader_open(struct binary_reader_context *context)
{
	if (context->fp != NULL) {
		return -1;
	}

	if ((context->fp = fopen(context->file_path, "r+b")) == NULL) {
		printf("Error opening file %s: %s\n", context->file_path, strerror(errno));
		return -1;
	}

	rewind(context->fp);

	return 0;
}

int binary_reader_read_boolean(struct binary_reader_context *context, bool *out_value)
{
	return binary_reader_read_byte(context, (uint8_t *)out_value);
}

int binary_reader_read_byte(struct binary_reader_context *context, uint8_t *out_value)
{
	fread(out_value, 1, 1, context->fp);

	//if (fread(out_value, 1, 1, context->fp) != 1) {
	//	if (feof(context->fp)) {
	//		_ERROR("%s: EOF reading file %s at position %ld\n", __FUNCTION__,
	//			context->file_path, ftell(context->fp));
	//	}
	//	else if (ferror(context->fp)) {
	//		_ERROR("%s: IO error reading file %s at position %ld\n", __FUNCTION__,
	//			context->file_path, ftell(context->fp));
	//	}
	//	return -1;
	//}

	//if (out_value != NULL) {
	//	*out_value = val;
	//}

	return 0;
}

int binary_reader_read_decimal(struct binary_reader_context *context, long double *out_value);

int binary_reader_read_double(struct binary_reader_context *context, double *out_value)
{
	double val;

	if (fread(&val, sizeof(double), 1, context->fp) != 1) {
		return -1;
	}

	if (out_value != NULL) {
		*out_value = val;
	}

	return 0;
}

int binary_reader_read_int16(struct binary_reader_context *context, int16_t *out_value)
{
	uint8_t buffer[2];

	if (fread(buffer, sizeof(int16_t), 1, context->fp) != 1) {
		return -1;
	}

	*out_value = le16_to_cpu(buffer);

	return 0;
}

int binary_reader_read_int32(struct binary_reader_context *context, int32_t *out_value)
{
	int32_t val;
	size_t items;

	if ((items = fread(&val, sizeof(int32_t), 1, context->fp)) != 1) {
		if (feof(context->fp)) {
			_ERROR("%s: EOF reading file %s at position %ld\n", __FUNCTION__,
				context->file_path, ftell(context->fp));
		}
		else if (ferror(context->fp)) {
			_ERROR("%s: IO error reading file %s at position %ld\n", __FUNCTION__,
				context->file_path, ftell(context->fp));
		};

		return -1;
	}

	if (out_value != NULL) {
		*out_value = val;
	}

	return 0;
}

int binary_reader_read_int64(struct binary_reader_context *context, int64_t *out_value)
{
	int64_t val;

	if (fread(&val, sizeof(int64_t), 1, context->fp) != 1) {
		return -1;
	}

	if (out_value != NULL) {
		*out_value = val;
	}

	return 0;
}

int binary_reader_read_single(struct binary_reader_context *context, float *out_value)
{
	float val;

	if (fread(&val, sizeof(float), 1, context->fp) != 1) {
		return -1;
	}

	if (out_value != NULL) {
		*out_value = val;
	}

	return 0;
}

int binary_reader_read_string_buffer(char *buf, int pos, int *out_len, char **out_value)
{
	if (binary_reader_read_7bit_int(buf, &pos, &out_len) < 0) {
		return -1;
	}

	*out_value = (char *)(buf + pos);
}

int binary_reader_read_string(struct binary_reader_context *context, char **out_value)
{
	char *val;
	size_t string_length = 0;
	int ret;

	if (__read_7_bit_int(context, (int32_t *)&string_length) < 0) {
		return -1;
	}

	/*
	 * Note:
	 *
	 * .NET strings have a 7-bit encoded length at the start
	 * and no null terminator.  Must allocate enough room for
	 * the null terminator at the end of the c string.
	 */
	if ((val = (char *)malloc(string_length + 1)) == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	if (fread(val, string_length, 1, context->fp) != 1) {
		ret = -1;
		goto failed;
	}

	val[string_length] = '\0';

	if (out_value != NULL) {
		*out_value = val;
	}

	ret = 0;
	goto out;

failed:
	free(val);
out:
	return ret;
}

int binary_reader_read_uint16(struct binary_reader_context *context, uint16_t *out_value)
{
	uint16_t val;

	if (fread(&val, sizeof(uint16_t), 1, context->fp) != 1) {
		return -1;
	}

	if (out_value != NULL) {
		*out_value = val;
	}

	return 0;
}

int binary_reader_read_uint32(struct binary_reader_context *context, uint32_t *out_value)
{
	uint32_t val;

	if (fread(&val, sizeof(uint32_t), 1, context->fp) != 1) {
		return -1;
	}

	if (out_value != NULL) {
		*out_value = val;
	}

	return 0;
}

int binary_reader_read_uint64(struct binary_reader_context *context, uint64_t *out_value)
{
	uint64_t val;

	if (fread(&val, sizeof(uint64_t), 1, context->fp) != 1) {
		return -1;
	}

	if (out_value != NULL) {
		*out_value = val;
	}

	return 0;
}

int binary_reader_close(struct binary_reader_context *context)
{
	if (context->fp == NULL) {
		return -1;
	}

	fclose(context->fp);
	return 0;
}

int binary_reader_read_7bit_int(const char *buffer, int *pos, int32_t *out_value)
{
	int count = 0, shift = 0;
	uint8_t byte;

	do {
		if (shift == 5 * 7) {
			return -1;
		}

		if ((byte = buffer[*pos]) < 0) {
			return -1;
		}

		count |= (byte & 0x7F) << shift;
		shift += 7;
		(*pos)++;
	} while ((byte & 0x80) != 0);

	*out_value = count;

	return 0;
}