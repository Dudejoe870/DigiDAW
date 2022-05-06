cmake_minimum_required (VERSION 3.8)

project ("DigiDAWCore")

option(DIGIDAW_COMPILE_WITH_AVX "Whether or not to build with AVX support" ON)
option(DIGIDAW_AVX2 "Whether or not to use AVX2 when compiling with AVX" ON)

add_library (DigiDAWCore STATIC "src/audio/engine.cpp" "src/audio/mixer.cpp" "src/audio/trackstate.cpp")

if (DIGIDAW_COMPILE_WITH_AVX AND NOT DIGIDAW_AVX2)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(DigiDAWCore PRIVATE "-mavx")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(DigiDAWCore PRIVATE "/arch:AVX")
    endif()

    add_compile_definitions(SIMDPP_ARCH_X86_AVX)
elseif(DIGIDAW_COMPILE_WITH_AVX AND DIGIDAW_AVX2)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(DigiDAWCore PRIVATE "-mavx2")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(DigiDAWCore PRIVATE "/arch:AVX2")
    endif()

    add_compile_definitions(SIMDPP_ARCH_X86_AVX2)
endif()

set_property(TARGET DigiDAWCore PROPERTY CXX_STANDARD 20)

target_link_libraries(DigiDAWCore PUBLIC rtaudio)

target_include_directories(DigiDAWCore 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/priv_include" 
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}../rtaudio")
