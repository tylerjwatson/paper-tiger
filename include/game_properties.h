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

* paper-tiger is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with paper-tiger.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
  * Describes the configuration that makes up a paper tiger server instance.
  *
  * After calling ptGameSetProperties(), the memory occupied by this structure
  * must live for the life of the game instance.
  */
typedef struct ptGameProperties {
    /** Specifies how often the game engine ticks in milliseconds */
    double msPerFrame;
    /** Specifies a path to the world file. */
    char* worldFilePath;
    /** Specifies the maximum number of players on a world at once */
    uint8_t maxPlayers;
    /** Specifies whether to enable captive console output */
    bool enableConsole;

    char* listenAddr;

    uint16_t listenPort;
} ptGameProperties;

/**
  * Populates a ptGameProperties structure pointed to by @a gameProperties
  * with sane default values.
  *
  * Caller owns all memory allocated by this function.
  */
void
ptGamePropertiesDefaultProperties(ptGameProperties* gameProperties);

#ifdef __cplusplus
}
#endif
