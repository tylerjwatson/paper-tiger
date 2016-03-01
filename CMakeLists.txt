cmake_minimum_required(VERSION 2.6)
project(upgraded-guacamole C)

# set(CMAKE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")



# include(FindTalloc)
include(FindLibUV)

# include_directories(${TALLOC_INCLUDE_DIRS})

if(NOT LIBUV_FOUND)
	message(FATAL_ERROR "LibUV not found and is required.")
endif()

add_library(talloc STATIC src/talloc/talloc.c)

include_directories("/usr/local/include" "${PROJECT_SOURCE_DIR}" "${LIBUV_INCLUDE_DIRS}") 

add_executable(upgraded-guacamole src/getopt.c src/world.c src/tile.c src/binary_reader.c src/server.c src/game.c src/player.c src/main.c)

set_property(TARGET upgraded-guacamole PROPERTY C_STANDARD 11)
target_link_libraries(upgraded-guacamole talloc "${LIBUV_LIBRARIES}")

install(TARGETS upgraded-guacamole RUNTIME DESTINATION bin)

set_target_properties(upgraded-guacamole PROPERTIES OUTPUT_NAME "guac")
