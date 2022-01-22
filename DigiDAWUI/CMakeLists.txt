cmake_minimum_required (VERSION 3.8)

project ("DigiDAWUI")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32)
    set(CMAKE_PREFIX_PATH "$ENV{QTDIR}/lib/cmake")
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(DigiDAWUI "resources.qrc" "src/mainwindow.ui" "src/settings.ui" "src/about.ui" "src/settings.h" "src/settings.cpp" "src/mainwindow.cpp" "src/mainwindow.h" "src/main.cpp" "src/main.h" "src/about.h" "src/about.cpp")

add_custom_command(
    TARGET DigiDAWUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:DigiDAWUI>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:DigiDAWUI>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:DigiDAWUI>)

if (WIN32)
    add_custom_command(
        TARGET DigiDAWUI POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_directory "$ENV{QTDIR}/plugins/platforms" $<TARGET_FILE_DIR:DigiDAWUI>/platforms)
endif()

set_property(TARGET DigiDAWUI PROPERTY OUTPUT_NAME "DigiDAW")

set_property(TARGET DigiDAWUI PROPERTY CXX_STANDARD 20)
set_property(TARGET DigiDAWUI PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(DigiDAWUI PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets DigiDAWCore)
