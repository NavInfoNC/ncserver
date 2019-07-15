cmake_minimum_required(VERSION 2.6)
project(echo)

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

FILE(GLOB ECHO_SOURCE "${SOURCE_PATH}/*.cpp")

message(STATUS ${ECHO_SOURCE})

IF (${CMAKE_GENERATOR} MATCHES "^Visual Studio 12.*Win64.*$") 

	SET (CMAKE_CXX_FLAGS_DEBUG "/MT /NODEFAULTLIB:\"msvcrt.lib\"")
	SET (CMAKE_CXX_FLAGS_RELEASE "/MT /NODEFAULTLIB:\"msvcrt.lib\"")

	add_definitions(
		-DAMD64 -D_CRT_SECURE_NO_WARNINGS
	)

	add_executable(echo_service ${ECHO_SOURCE})

	IF (${CMAKE_BUILD_TYPE} STREQUAL "Release")
		target_link_libraries(echo_service
			ncserver_x64.lib
		)
	ELSE()
		target_link_libraries(echo_service
			ncserver_x64_d.lib
		)
	ENDIF()

ELSEIF (${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")

	add_executable(echo_service.out ${ECHO_SOURCE})

	target_link_libraries(echo_service.out
		m
		rt
		dl
		ncserver
		dl
		rt
		m
	)

ENDIF ()
