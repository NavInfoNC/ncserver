cmake_minimum_required(VERSION 2.6)
project(ncserver)

set (BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set (SOURCE_PATH ${BASE_PATH}/src)
set (LIBRARY_OUTPUT_PATH ${BASE_PATH}/lib)

include_directories(
	${BASE_PATH}/include
	${BASE_PATH}/fastcgi/include
)

link_directories ("${BASE_PATH}/dependency/lib")

SET (CMAKE_CXX_FLAGS "-std=c++11")
SET (LIB_SUFFIX "")
set (DEPENDENT_LIB
	libyaml-cpp.a
	)	
add_definitions(
	-DNDEBUG
	-DLINUX
	-DAMD64
	-g
	-fshort-wchar
	-fwide-exec-charset=UTF-16LE
	-m64
)

FILE(GLOB SRC "${BASE_PATH}/src/*.cpp" "${BASE_PATH}/src/*.c")
FILE(GLOB FCGI_SRC "${BASE_PATH}/fastcgi/libfcgi/*.cpp" "${BASE_PATH}/fastcgi/libfcgi/*.c")

add_library(ncserver${LIB_SUFFIX} STATIC ${SRC} ${FCGI_SRC})
target_link_libraries(ncserver${LIB_SUFFIX} ${DEPENDENT_LIB})

ADD_SUBDIRECTORY (example)
ADD_SUBDIRECTORY (test)
