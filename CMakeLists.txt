cmake_minimum_required(VERSION 3.1)
project(paper-tiger)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")

if(NOT CMAKE_LIBRARY_ARCHITECTURE)
	set(CMAKE_LIBRARY_ARCHITECTURE "x86")
endif()

if(WIN32)
	set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/win32")
	include(FindLibUV-Win32)
else()
	include(FindLibUV)
endif()

include(FindZLIB)

if(NOT ZLIB_FOUND)
	message(FATAL_ERROR "zlib not found and is required.")
endif()

if(NOT LIBUV_FOUND)
	message(FATAL_ERROR "LibUV not found and is required.")
endif()

add_library(talloc STATIC src/talloc/talloc.c)

include_directories("/usr/local/include" "${PROJECT_SOURCE_DIR}" "${LIBUV_INCLUDE_DIRS}" "${ZLIB_INCLUDE_DIRS}") 

add_executable(paper-tiger
	src/packet.c
	src/packet.h

	src/packets/client_uuid.h
	src/packets/client_uuid.c
	src/packets/continue_connecting2.h
	src/packets/continue_connecting2.c
	src/packets/continue_connecting.h
	src/packets/continue_connecting.c
	src/packets/player_info.h
	src/packets/player_info.c
	src/packets/connect_request.h
	src/packets/connect_request.c
	src/packets/world_info.c
	src/packets/world_info.h
	src/packets/disconnect.c
	src/packets/disconnect.h
	src/packets/get_section.c
	src/packets/get_section.h
	src/packets/player_hp.h
	src/packets/player_hp.c
	src/packets/player_mana.h
	src/packets/player_mana.c

	src/rect.h

	src/binary_writer.c
	src/binary_writer.h
	src/binary_reader.c
	src/binary_reader.h
	src/bitmap.h
	src/param.cc
	src/param.h
	src/console.c
	src/console.h
	src/game.c
	src/game.h
	src/getopt.c
	src/getopt.h
	src/main.cc
	src/player.c
	src/player.h
	src/server.c
	src/server.h
	src/tile.c
	src/tile.h
	src/util.h
	src/world.c
	src/world.h)

set_property(TARGET paper-tiger PROPERTY C_STANDARD 11)

if(WIN32)
	target_link_libraries(paper-tiger talloc "${LIBUV_LIBRARIES}" "${ZLIB_LIBRARIES}")
else()
	target_link_libraries(paper-tiger talloc "${LIBUV_LIBRARIES}" "${ZLIB_LIBRARIES}")
endif()

install(TARGETS paper-tiger RUNTIME DESTINATION bin)
