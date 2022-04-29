cmake_minimum_required (VERSION 3.8)

project ("DigiDAWCore")

add_library (DigiDAWCore STATIC "src/stdcommon.h" "src/audio/engine.cpp" "src/audio/common.h" "src/stdcommon.h" "src/audio/engine.h" "src/audio/mixer.cpp" "src/audio/mixer.h" "src/audio/trackstate.h" "src/audio/trackstate.cpp")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(DigiDAWCore PRIVATE "-mavx")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(DigiDAWCore PRIVATE "/arch:AVX")
endif()

set_property(TARGET DigiDAWCore PROPERTY CXX_STANDARD 20)

target_link_libraries(DigiDAWCore PUBLIC rtaudio)

target_include_directories(DigiDAWCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}../rtaudio")
