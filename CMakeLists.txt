cmake_minimum_required(VERSION 3.15)
project(PTGPU)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(ext/imgui/ ext/glew/include)

add_executable(PTGPU main.cpp)