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

#pragma once

#include "talloc/talloc.h"

#define VECTOR_INITIAL_CAPACITY 1

#ifdef __cplusplus
extern "C" {
#endif

struct vector {
	size_t size;
	size_t capacity;
	void **data;
};

int
vector_new(TALLOC_CTX *context, struct vector **out_vector);

int
vector_steal_back(struct vector *vector, void *data);

int
vector_push_back(struct vector *vector, void *data);

void *
vector_get(struct vector *vector, size_t index);

void
vector_steal(struct vector *vector, size_t index, void *data);

void
vector_set(struct vector *vector, size_t index, void *data);

void
vector_delete(struct vector *vector, void *ptr);

#ifdef __cplusplus
}
#endif