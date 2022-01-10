cmake_minimum_required (VERSION 3.8)

# TODO: Make this cross-platform (should be relatively easy, but I'm going to start working on the main project now.)
add_executable (DigiDAW WIN32 "src/main.cpp" "src/resources.cpp")
target_sources(DigiDAW PRIVATE "$ENV{SCITERSDK}/include/sciter-win-main.cpp")

target_link_libraries(DigiDAW rtaudio)

target_include_directories(DigiDAW PRIVATE "$ENV{SCITERSDK}/include" "src" "${CMAKE_CURRENT_SOURCE_DIR}../rtaudio")
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.cpp
  COMMAND $ENV{SCITERSDK}/bin/windows/packfolder.exe ui src/resources.cpp -v "resources"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ui ${CMAKE_CURRENT_SOURCE_DIR}/ui/statusbar ${CMAKE_CURRENT_SOURCE_DIR}/ui/menubar
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
  COMMENT "Packing Resources into resources.cpp..."
)
add_custom_command(
    TARGET DigiDAW POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows" $<TARGET_FILE_DIR:DigiDAW>
    VERBATIM
)
set_property(TARGET DigiDAW PROPERTY CXX_STANDARD 20)
