cmake_minimum_required (VERSION 3.8)

project ("DigiDAWUI")

set(RESOURCE_FILE_SOURCE "src/res/resources.c")
set(RESOURCE_FILE_HEADER "include/res/resources.h")

# From https://stackoverflow.com/a/27206982 (Modified)
# Creates C resources file from files in given directory
function(create_resources dir src_output header_output)
    # Create empty output files
    file(WRITE ${src_output} 
"/** This is an Auto Generated File by DigiDAWUI/CMakeLists.txt **/
")
    file(WRITE ${header_output} 
"/** This is an Auto Generated File by DigiDAWUI/CMakeLists.txt **/
#pragma once

namespace DigiDAW::UI::Resources
{")

    # Collect input files
    file(GLOB bins ${dir}/*)
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        set(filename_non_c ${filename})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${src_output} "
// ${dir}/${filename_non_c}
const unsigned char ${filename}[] = {${filedata}};
const unsigned ${filename}_size = sizeof(${filename});
")
        file(APPEND ${header_output} "
    // ${dir}/${filename_non_c}
    extern \"C\" const unsigned char ${filename}[];
    extern \"C\" const unsigned ${filename}_size;
")
    endforeach()

    file(APPEND ${header_output} "} // namespace DigiDAW::UI::Resources\n")
endfunction()

create_resources("res" ${RESOURCE_FILE_SOURCE} ${RESOURCE_FILE_HEADER})

add_executable(DigiDAWUI "src/main.cpp" "src/glad/glad.c" "src/imgui/imgui_impl_glfw.cpp" "src/imgui/imgui.cpp" "src/imgui/imgui_demo.cpp" "src/imgui/imgui_draw.cpp" "src/imgui/imgui_tables.cpp" "src/imgui/imgui_widgets.cpp" "src/imgui/imgui_impl_opengl3.cpp" "src/ui.cpp" "include/ui.h"  ${RESOURCE_FILE_SOURCE} "include/gui_util.h" "src/gui_util.cpp")
target_include_directories(DigiDAWUI PRIVATE "src" "include")

set_property(TARGET DigiDAWUI PROPERTY OUTPUT_NAME "DigiDAW")

set_property(TARGET DigiDAWUI PROPERTY CXX_STANDARD 20)
set_property(TARGET DigiDAWUI PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(DigiDAWUI PUBLIC DigiDAWCore glfw fmt)
