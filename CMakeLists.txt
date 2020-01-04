cmake_minimum_required(VERSION 3.15)
project(PTGPU)

set(CMAKE_CXX_STANDARD 17)

find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
#find_package(OpenCL REQUIRED)

link_directories(lib)

set(LIBRARIES glfw GLEW::glew imgui)

set(SOURCE_FILES
        # Header files
        include/CLCompiler.h
        include/CLTracer.h
        include/Display.h
        include/imgui.h
        include/imconfig.h
        include/imgui_internal.h
        include/imstb_rectpack.h
        include/imstb_textedit.h
        include/imstb_truetype.h
        include/imgui_stdlib.h
        include/imgui_impl_glfw.h
        include/imgui_impl_opengl3.h

        # Source files
        src/main.cpp
        src/CLCompiler.cpp
        src/CLTracer.cpp
        src/Display.cpp)


add_executable(PTGPU ${SOURCE_FILES})
target_link_libraries(PTGPU ${LIBRARIES})
