cmake_minimum_required (VERSION 3.8)

project ("DigiDAW")

option(DIGIDAW_BUILD_UI "Whether or not to build the UI" ON)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} MATCHES "CYGWIN_NT*" OR ${CMAKE_SYSTEM_NAME} MATCHES "MSYS_NT*")
	set(RTAUDIO_API_DS ON)
	set(RTAUDIO_API_ASIO ON)
	set(RTAUDIO_API_WASAPI ON)
	set(RTAUDIO_STATIC_MSVCRT OFF)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(RTAUDIO_API_JACK ON)
	set(RTAUDIO_API_PULSE ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	set(RTAUDIO_API_CORE ON)
endif()

set(BUILD_SHARED_LIBS OFF)

set(RTAUDIO_BUILD_TESTING OFF)
set(RTAUDIO_BUILD_SHARED_LIBS OFF)
set(RTAUDIO_BUILD_STATIC_LIBS ON)
add_subdirectory ("rtaudio" EXCLUDE_FROM_ALL)

add_subdirectory ("DigiDAWCore")

if (DIGIDAW_BUILD_UI)
	set(GLFW_INSTALL OFF)
	set(GLFW_BUILD_DOCS OFF)
	set(GLFW_LIBRARY_TYPE "STATIC")
	add_subdirectory ("glfw" EXCLUDE_FROM_ALL)

	add_subdirectory ("fmt" EXCLUDE_FROM_ALL)

	add_subdirectory ("DigiDAWUI")
endif()
