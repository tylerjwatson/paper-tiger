
find_path(LIBUV_INCLUDE_DIRS uv.h)

find_library(LIBUV_LIBRARIES 
	NAMES uv libuv
	HINTS "${CMAKE_PREFIX_PATH}/lib")

if(LIBUV_INCLUDE_DIRS AND LIBUV_LIBRARIES)
	set(LIBUV_FOUND true)
endif()