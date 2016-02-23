cmake_minimum_required(VERSION 2.6)
project(upgraded-guacamole C)

set(CMAKE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")

include_directories("/usr/local/include")

message(STATUS "${PROJECT_SOURCE_DIR}")

include(FindTalloc)

include_directories(${TALLOC_INCLUDE_DIRS})

add_executable(upgraded-guacamole src/world.c src/tile.c src/binary_reader.c src/game.c src/main.c)

target_link_libraries(upgraded-guacamole ${TALLOC_LIBRARIES})


install(TARGETS upgraded-guacamole RUNTIME DESTINATION bin)

set_target_properties(upgraded-guacamole PROPERTIES OUTPUT_NAME "guac")
