cmake_minimum_required (VERSION 3.8)

project ("DigiDAW")

set(RTAUDIO_API_DS ON)
set(RTAUDIO_API_ASIO ON)
set(RTAUDIO_BUILD_TESTING OFF)
set(RTAUDIO_BUILD_SHARED_LIBS OFF)
set(RTAUDIO_BUILD_STATIC_LIBS ON)
set(RTAUDIO_STATIC_MSVCRT OFF)
add_subdirectory ("rtaudio")

add_subdirectory ("DigiDAW")
