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

* upgraded-guacamole is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with upgraded-guacamole.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _HAVE_CONFIG_H
#define _HAVE_CONFIG_H

#define PRODUCT_NAME "paper-tiger"
#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#ifdef _WIN32
#define PRODUCT_PLATFORM Windows
#elif __APPLE__
#define PRODUCT_PLATFORM Apple Mac
#elif __linux__
#define PRODUCT_PLATFORM Linux
#else
#define PRODUCT_PLATFORM Unknown
#endif

#endif //_HAVE_RECT_H
