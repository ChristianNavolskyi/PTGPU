cmake_minimum_required(VERSION 3.15)
project(PTGPU)

#set(CMAKE_CXX_COMPILER /usr/local/Cellar/gcc/9.2.0_2/bin/gcc-9)
set(CMAKE_CXX_STANDARD 17)

#find_package(GLEW REQUIRED)
#find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)

#include_directories(ext/imgui)

SET(LIBRARIES ${OPENGL_LIBRARY} ${OPENCL_LIBRARY})

add_executable(PTGPU main.cpp)
target_link_libraries(PTGPU ${LIBRARIES})
