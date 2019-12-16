cmake_minimum_required(VERSION 2.6)
set (PROJ_NAME echo)
project(${PROJ_NAME})

set (BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
message(STATUS ${BASE_PATH})
set (SOURCE_PATH .)
set (INCLUDE_PATH ${BASE_PATH}/include)
set (LIB_PATH ${BASE_PATH}/lib)

set (EXECUTABLE_OUTPUT_PATH ${LIB_PATH})

include_directories(
	${BASE_PATH}
	${INCLUDE_PATH}
	${LIB_PATH}
)

link_directories(
	${LIB_PATH}/${CMAKE_CONFIG}
)

FILE(GLOB SOURCE "${SOURCE_PATH}/*.cpp")

message(STATUS ${SOURCE})

add_executable(${PROJ_NAME} ${SOURCE})

target_link_libraries(${PROJ_NAME}
	m
	rt
	dl
	ncserver
	pthread
	dl
	rt
	m
)
