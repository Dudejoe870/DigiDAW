cmake_minimum_required (VERSION 3.8)

# Get all subdirectories recursively.
macro(subdirlist result curdir)
    file(GLOB_RECURSE children LIST_DIRECTORIES true RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            list(APPEND dirlist ${curdir}/${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

add_executable (DigiDAW "src/ui/uimain.cpp" "src/ui/resources.cpp" "src/ui/mainwindow.cpp" "src/ui/mainwindow.h" "src/stdcommon.h" "src/ui/digidaw.h" "src/ui/registry.h" "src/ui/registry.cpp" "src/ui/digidaw.cpp" "src/audio/engine.cpp" "src/audio/common.h" "src/stdcommon.h" "src/audio/engine.h" "src/main.h" "src/main.cpp" "src/ui/audioengine.h" "src/ui/audioengine.cpp" "src/audio/mixer.cpp" "src/audio/mixer.h" "src/ui/audiomixer.h" "src/ui/audiomixer.cpp" "src/audio/trackstate.h" "src/audio/trackstate.cpp")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(DigiDAW PRIVATE "-mavx")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(DigiDAW PRIVATE "/arch:AVX")
endif()

set_property(TARGET DigiDAW PROPERTY CXX_STANDARD 20)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} MATCHES "CYGWIN_NT*" OR ${CMAKE_SYSTEM_NAME} MATCHES "MSYS_NT*")
    set_property(TARGET DigiDAW PROPERTY WIN32_EXECUTABLE TRUE)
    target_sources(DigiDAW PRIVATE "$ENV{SCITERSDK}/include/sciter-win-main.cpp")

    set(SCITER_PLATFORM_DIRECTORY "windows")
    set(SCITER_LIB_DIRECTORY "x64")
    set(SCITER_LIB_NAME "sciter.dll")
    set(EXE_EXT ".exe")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_sources(DigiDAW PRIVATE "$ENV{SCITERSDK}/include/sciter-gtk-main.cpp")

    set(SCITER_PLATFORM_DIRECTORY "linux")
    set(SCITER_LIB_DIRECTORY "x64")
    set(SCITER_LIB_NAME "libsciter-gtk.so")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

    target_compile_options(DigiDAW PRIVATE ${GTK3_CFLAGS_OTHER})
    target_include_directories(DigiDAW PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_directories(DigiDAW PRIVATE ${GTK3_LIBRARY_DIRS})
    target_link_libraries(DigiDAW ${GTK3_LIBRARIES} dl)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_sources(DigiDAW PRIVATE "$ENV{SCITERSDK}/include/sciter-osx-main.mm")

    set(SCITER_PLATFORM_DIRECTORY "macosx")
    set(SCITER_LIB_DIRECTORY ".")
    set(SCITER_LIB_NAME "libsciter.dylib")
endif()

target_link_libraries(DigiDAW rtaudio)

target_include_directories(DigiDAW PUBLIC "$ENV{SCITERSDK}/include" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}../rtaudio")

subdirlist(UI_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/resources.cpp
  COMMAND "$ENV{SCITERSDK}/bin/${SCITER_PLATFORM_DIRECTORY}/packfolder${EXE_EXT}" ui src/ui/resources.cpp -v "resources"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ui ${UI_DIRS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
  COMMENT "Packing Resources into resources.cpp..."
)

add_custom_command(
    TARGET DigiDAW POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$ENV{SCITERSDK}/bin/${SCITER_PLATFORM_DIRECTORY}/${SCITER_LIB_DIRECTORY}/${SCITER_LIB_NAME}" $<TARGET_FILE_DIR:DigiDAW>
    VERBATIM
)
