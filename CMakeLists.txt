CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(chat)

SET(CMAKE_C_STANDARD 99)
SET(CMAKE_C_STANDARD_REQUIRED ON)
IF("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
		"${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
	# Special flags for debug builds. Should eventually be changed to
	# -Og -ggdb when everyone has a compiler supporting it.
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb")
ENDIF()

SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
SET(THREADS_PREFER_PTHREAD_FLAG TRUE)
FIND_PACKAGE(Threads REQUIRED)

FIND_LIBRARY(LIBRT rt)

SET(SERVER_MODULES	src/broadcastagent.c
			src/clientthread.c
			src/connectionhandler.c
			src/main.c
			src/network.c
			src/user.c
			src/util.c)

INCLUDE_DIRECTORIES(src)

ADD_EXECUTABLE(server ${SERVER_MODULES})
IF(LIBRT)
	TARGET_LINK_LIBRARIES(server Threads::Threads ${LIBRT})
ELSE()
	TARGET_LINK_LIBRARIES(server Threads::Threads)
ENDIF()
