cmake_minimum_required(VERSION 2.6)
project(ncserver)

set (BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set (SOURCE_PATH ${BASE_PATH}/src)
set (LIBRARY_OUTPUT_PATH ${BASE_PATH}/lib)

include_directories(
	${BASE_PATH}/include
	${BASE_PATH}/fastcgi/include
)

IF (${CMAKE_GENERATOR} MATCHES "^Visual Studio 12.*Win64.*$") 

	IF (${CMAKE_BUILD_TYPE} STREQUAL "Release")
		SET (CMAKE_CXX_FLAGS_DEBUG "/MT /O1")
		SET (CMAKE_CXX_FLAGS_RELEASE "/MT /O1")
		SET (LIB_SUFFIX "_x64")
	ELSE()
		SET (CMAKE_CXX_FLAGS_DEBUG "/MT")
		SET (CMAKE_CXX_FLAGS_RELEASE "/MT")
		SET (LIB_SUFFIX "_x64_d")
	ENDIF()

	add_definitions(
		-DAMD64 -D_CRT_SECURE_NO_WARNINGS
	)

ELSEIF (${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")

	SET (CMAKE_CXX_FLAGS "-std=c++11")
	SET (LIB_SUFFIX "")
		
	add_definitions(
		-DNDEBUG
		-DLINUX
		-DAMD64
		-g
		-fshort-wchar
		-fwide-exec-charset=UTF-16LE
		-m64
	)
ELSE ()

	MESSAGE (FATAL_ERROR "Unsupported Generator")
	
ENDIF()

FILE(GLOB SRC "${BASE_PATH}/src/*.cpp" "${BASE_PATH}/src/*.c")
FILE(GLOB FCGI_SRC "${BASE_PATH}/fastcgi/libfcgi/*.cpp" "${BASE_PATH}/fastcgi/libfcgi/*.c")

add_library(ncserver${LIB_SUFFIX} STATIC ${SRC} ${FCGI_SRC})

ADD_SUBDIRECTORY (example)
