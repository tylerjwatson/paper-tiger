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

include_directories("${PROJECT_SOURCE_DIR}/include" 
					"${LIBUV_INCLUDE_DIRS}" 
					"${ZLIB_INCLUDE_DIRS}")				


add_executable(paper-tiger
	src/game.c
	src/game.h
	src/packet.c
	src/packet.h

	src/packets/status.c
	src/packets/status.h
	src/packets/world_info.c
	src/packets/world_info.h
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
	src/packets/disconnect.c
	src/packets/disconnect.h
	src/packets/get_section.c
	src/packets/get_section.h
	src/packets/player_hp.h
	src/packets/player_hp.c
	src/packets/player_mana.h
	src/packets/player_mana.c
	src/packets/tile_section.h
	src/packets/tile_section.c
	src/packets/section_tile_frame.c
	src/packets/section_tile_frame.h
	src/packets/inventory_slot.c
	src/packets/inventory_slot.h
	src/packets/chat_message.c
	src/packets/chat_message.h
	src/packets/connection_complete.c
	src/packets/connection_complete.h

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
	src/world_section.c
	src/world_section.h
	src/world.c
	src/world.h
	src/dataloader.h
	src/dataloader.cc

	src/vector.c
	src/vector.h
	src/hook.c
	src/hook.h

	${PROTO_SRCS}
	${PROTO_HEADERS})
	
set_property(TARGET paper-tiger PROPERTY C_STANDARD 11)

target_link_libraries(paper-tiger 
	talloc 
	"${LIBUV_LIBRARIES}"
	"${ZLIB_LIBRARIES}")

install(TARGETS paper-tiger RUNTIME DESTINATION bin)
