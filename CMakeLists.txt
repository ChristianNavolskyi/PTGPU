cmake_minimum_required(VERSION 3.15)
project(PTGPU)

set(CMAKE_CXX_STANDARD 17)

find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
#find_package(OpenCL REQUIRED)

add_subdirectory(ext/imgui)

set(LIBRARIES glfw GLEW::glew imgui)

set(SOURCE_FILES
        # Header files
        include/CLCompiler.h
        include/CLTracer.h
        include/Display.h
        include/Renderer.h

        # Source files
        src/main.cpp
        src/CLCompiler.cpp
        src/CLTracer.cpp
        src/Display.cpp
        src/Renderer.cpp)

add_executable(PTGPU ${SOURCE_FILES})
target_link_libraries(PTGPU ${LIBRARIES})
target_include_directories(PTGPU PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/ext/imgui/include)
