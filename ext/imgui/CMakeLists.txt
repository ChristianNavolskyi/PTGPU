cmake_minimum_required(VERSION 3.15)
project(imgui)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)

set(IMGUI_SOURCE_FILES

    # Header files
    include/imgui/imgui.h
    include/imgui/imconfig.h
    include/imgui/imgui_internal.h
    include/imgui/imstb_rectpack.h
    include/imgui/imstb_textedit.h
    include/imgui/imstb_truetype.h
    include/imgui/imgui_stdlib.h
    include/imgui/imgui_impl_glfw.h
    include/imgui/imgui_impl_opengl3.h

    # Source files
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_stdlib.cpp
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    )

add_library(imgui STATIC ${IMGUI_SOURCE_FILES})
target_link_libraries(imgui glfw GLEW::glew)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_DIR}/include)