
find_path(LIBUV_INCLUDE_DIRS uv.h)

find_library(LIBUV_LIBRARY 
	NAMES uv libuv
	HINTS "${CMAKE_PREFIX_PATH}/lib")

if(LIBUV_INCLUDE_DIRS AND LIBUV_LIBRARY)
	set(LIBUV_FOUND true)
endif()