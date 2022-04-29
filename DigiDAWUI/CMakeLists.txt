cmake_minimum_required (VERSION 3.8)

project ("DigiDAWUI")

add_executable(DigiDAWUI "src/main.cpp" "src/glad/glad.c" "src/imgui/imgui_impl_glfw.cpp" "src/imgui/imgui.cpp" "src/imgui/imgui_demo.cpp" "src/imgui/imgui_draw.cpp" "src/imgui/imgui_tables.cpp" "src/imgui/imgui_widgets.cpp" "src/imgui/imgui_impl_opengl3.cpp" "src/ui.cpp" "include/ui.h")
target_include_directories(DigiDAWUI PRIVATE "src" "include")

set_property(TARGET DigiDAWUI PROPERTY OUTPUT_NAME "DigiDAW")

set_property(TARGET DigiDAWUI PROPERTY CXX_STANDARD 20)
set_property(TARGET DigiDAWUI PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(DigiDAWUI PUBLIC DigiDAWCore glfw)
