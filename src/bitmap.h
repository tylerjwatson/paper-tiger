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

#ifndef _HAVE_BITMAP_H
#define _HAVE_BITMAP_H

#include <limits.h>    /* for CHAR_BIT */
#include <stdint.h>   /* for uint32_t */
#include <stdbool.h>
typedef uint64_t word_t;

enum {
    BITS_PER_WORD = sizeof(word_t) * CHAR_BIT
};

#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

static inline void bitmap_set(word_t *words, int n)
{
    words[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

static inline void bitmap_clear(word_t *words, int n)
{
    words[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

static inline bool bitmap_get(word_t *words, int n)
{
    word_t bit = words[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
    return bit != 0;
}

#endif /* _HAVE_BITMAP_H */
