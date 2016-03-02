find_path(PTHREAD_INCLUDE_DIRS pthread.h)

find_library(PTHREAD_LIBRARY 
	NAMES pthread pthreadVC2
	HINTS "${CMAKE_PREFIX_PATH}/lib")

if(PTHREAD_INCLUDE_DIRS AND PTHREAD_LIBRARY)
	set(PTHREAD_FOUND true)
endif()