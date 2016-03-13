cmake_minimum_required(VERSION 2.6)
project(upgraded-guacamole)

# set(CMAKE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")

if(NOT CMAKE_LIBRARY_ARCHITECTURE)
	set(CMAKE_LIBRARY_ARCHITECTURE "x86")
endif()

if(WIN32)
	set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/win32")
	
	include(FindLibUV-Win32)
	# include(FindPThread-Win32)
else()
	include(FindLibUV)
endif()


if(NOT LIBUV_FOUND)
	message(FATAL_ERROR "LibUV not found and is required.")
endif()

#if (WIN32 AND NOT PTHREAD_FOUND)
#	message(FATAL_ERROR "pthread not found and is required.")
#endif()

add_library(talloc STATIC src/talloc/talloc.c)

include_directories("/usr/local/include" "${PROJECT_SOURCE_DIR}" "${LIBUV_INCLUDE_DIRS}") 

add_executable(upgraded-guacamole
	src/packets/connect_request.h
	src/packets/connect_request.c

	src/binary_reader.c
	src/binary_reader.h
	src/packet_type.h
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
	src/packet.c
	src/packet.h
	src/player.c
	src/player.h
	src/server.c
	src/server.h
	src/tile.c
	src/tile.h
	src/util.h
	src/world.c
	src/world.h)


set_property(TARGET upgraded-guacamole PROPERTY C_STANDARD 11)


if(WIN32)
	target_link_libraries(upgraded-guacamole talloc "${LIBUV_LIBRARIES}")
	# target_link_libraries(upgraded-guacamole talloc "${LIBUV_LIBRARIES}" "${PTHREAD_LIBRARY}")
else()
	target_link_libraries(upgraded-guacamole talloc "${LIBUV_LIBRARIES}")
endif()


install(TARGETS upgraded-guacamole RUNTIME DESTINATION bin)

set_target_properties(upgraded-guacamole PROPERTIES OUTPUT_NAME "guac")
