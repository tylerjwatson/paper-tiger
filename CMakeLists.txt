cmake_minimum_required(VERSION 2.6)
project(upgraded-guacamole C)

set(CMAKE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")

message(STATUS "${PROJECT_SOURCE_DIR}")

include(FindTalloc)

include_directories(${TALLOC_INCLUDE_DIRS})

add_executable(upgraded-guacamole src/binary_reader.c src/game.c src/main.c)

target_link_libraries(upgraded-guacamole ${TALLOC_LIBRARIES})



install(TARGETS upgraded-guacamole RUNTIME DESTINATION bin)
