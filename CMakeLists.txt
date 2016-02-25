cmake_minimum_required(VERSION 2.6)
project(upgraded-guacamole C)

# set(CMAKE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")

include_directories("/usr/local/include" "${PROJECT_SOURCE_DIR}") 


# include(FindTalloc)

# include_directories(${TALLOC_INCLUDE_DIRS})

add_library(talloc SHARED ccan/talloc/talloc.c)

add_executable(upgraded-guacamole src/world.c src/tile.c src/binary_reader.c src/game.c src/main.c)
set_property(TARGET upgraded-guacamole PROPERTY C_STANDARD 11)
target_link_libraries(upgraded-guacamole talloc)

install(TARGETS upgraded-guacamole RUNTIME DESTINATION bin)

set_target_properties(upgraded-guacamole PROPERTIES OUTPUT_NAME "guac")
