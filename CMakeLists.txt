cmake_minimum_required(VERSION 2.6)
project(ncserver)

set (BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set (SOURCE_PATH ${BASE_PATH}/src)
set (LIBRARY_OUTPUT_PATH ${BASE_PATH}/lib)

include_directories(
	${BASE_PATH}/include
	${BASE_PATH}/3rd-party/fastcgi/include
    ${BASE_PATH}/3rd-party/yaml-cpp/include 
)

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

FILE(GLOB SRC "${BASE_PATH}/src/*.cpp" "${BASE_PATH}/src/*.c")
FILE(GLOB FCGI_SRC "${BASE_PATH}/3rd-party/fastcgi/libfcgi/*.cpp" "${BASE_PATH}/3rd-party/fastcgi/libfcgi/*.c")
FILE(GLOB YAML_SRC "${BASE_PATH}/3rd-party/yaml-cpp/src/*.cpp" "${BASE_PATH}/3rd-party/yaml-cpp/src/*.c" "${BASE_PATH}/3rd-party/yaml-cpp/src/contrib/*.cpp")

add_library(ncserver${LIB_SUFFIX} STATIC ${SRC} ${FCGI_SRC} ${YAML_SRC})

ADD_SUBDIRECTORY (example)
ADD_SUBDIRECTORY (test)
