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

#include "vector.h"
#include "util.h"

// forward declaration;
static int
vector_realloc(struct vector *vector);

int
vector_new(TALLOC_CTX *context, struct vector **out_vector)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct vector *vec;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating temporary context for vector.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	vec = talloc_zero(temp_context, struct vector);
	if (vec == NULL) {
		_ERROR("%s: out of memory allocating temporary context for vector.\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	vec->capacity = 1;
	vec->size = 0;

	vector_realloc(vec);

	*out_vector = (struct vector *)talloc_steal(context, vec);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

static int
vector_realloc(struct vector *vector)
{
	size_t new_capacity = (vector->capacity << 1);

	vector->capacity = new_capacity;

	vector->data = talloc_realloc(vector, vector->data, void *, new_capacity);
	if (vector->data == NULL) {
		return -1;
	}

	return 0;
}

int
vector_steal_back(struct vector *vector, void *data)
{
	if (vector->size == vector->capacity) {
		if (vector_realloc(vector) < 0) {
			_ERROR("%s: out of memory reallocating vector array to %zu items.\n", __FUNCTION__, vector->capacity);
			return -1;
		}
	}

	vector_set(vector, vector->size++, data);

	return 0;
}

int
vector_push_back(struct vector *vector, void *data)
{
	if (vector->size >= vector->capacity) {
		if (vector_realloc(vector) < 0) {
			_ERROR("%s: out of memory reallocating vector array to %zu items.\n", __FUNCTION__, vector->capacity);
			return -1;
		}
	}

	vector_set(vector, vector->size++, data);

	return 0;
}

void *
vector_get(struct vector *vector, size_t index)
{
	if (index > vector->size) {
		return NULL;
	}

	return vector->data[index];
}

void
vector_steal(struct vector *vector, size_t index, void *data)
{
	if (index > vector->size) {
		return;
	}

	vector->data[index] = talloc_steal(vector->data, data);
}

void
vector_set(struct vector *vector, size_t index, void *data)
{
	if (index > vector->size) {
		return;
	}

	vector->data[index] = data;
}

void
vector_delete(struct vector *vector, void *ptr)
{
}