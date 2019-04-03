cmake_minimum_required(VERSION 3.1)
project(paper-tiger)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")


if(NOT CMAKE_LIBRARY_ARCHITECTURE)
	set(CMAKE_LIBRARY_ARCHITECTURE "x64")
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

include_directories("${PROJECT_SOURCE_DIR}/include"
                    "${PROJECT_SOURCE_DIR}/include/talloc" 
					"${LIBUV_INCLUDE_DIRS}" 
					"${ZLIB_INCLUDE_DIRS}")


add_library(talloc STATIC src/talloc/talloc.c)		

if(WIN32)
    add_library(mmap STATIC win32/mmap/windows-mmap.c)		
endif()


add_executable(paper-tiger
	src/game.c
	src/packet.c

	src/packets/status.c
	src/packets/world_info.c
	src/packets/client_uuid.c
	src/packets/continue_connecting2.c
	src/packets/continue_connecting.c
	src/packets/player_info.c
	src/packets/connect_request.c
	src/packets/disconnect.c
	src/packets/get_section.c
	src/packets/player_hp.c
	src/packets/player_mana.c
	src/packets/tile_section.c
	src/packets/section_tile_frame.c
	src/packets/inventory_slot.c
	src/packets/chat_message.c
	src/packets/connection_complete.c


	src/binary_writer.c
	src/binary_reader.c
	src/param.cc
	src/console.c
	src/getopt.c
	src/main.cc
	src/player.c
	src/server.c
	src/tile.c
	src/world_section.c
	src/world.c

	src/vector.c
	src/hook.c)
	
set_property(TARGET paper-tiger PROPERTY C_STANDARD 11)



if(WIN32)
    target_link_libraries(paper-tiger 
        mmap
        talloc
        ws2_32
        "${LIBUV_LIBRARIES}"
        "${ZLIB_LIBRARY_DEBUG}")
else()
    target_link_libraries(paper-tiger 
        talloc 
        "${LIBUV_LIBRARIES}"
        "${ZLIB_LIBRARIES}")
endif()


install(TARGETS paper-tiger RUNTIME DESTINATION bin)
