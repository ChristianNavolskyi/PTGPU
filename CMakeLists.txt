cmake_minimum_required(VERSION 3.15)
project(PTGPU)

set(CMAKE_CXX_STANDARD 17)

find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenCL REQUIRED)

find_package(GLUT)

add_subdirectory(ext/imgui)

set(LIBRARIES ${GLUT_LIBRARY} glfw GLEW::glew imgui ${OpenCL_LIBRARIES})

set(SOURCE_FILES
        # Header files
        include/CLCompiler.h
        include/CLTracer.h
        include/Renderer.h
        include/CLUtil.h
        include/Scene.h
        include/InteractiveCamera.h
        include/RenderInfoListener.h

        # Source files
        src/main.cpp
        src/CLCompiler.cpp
        src/CLTracer.cpp
        src/Renderer.cpp
        src/apple.c
        src/CLUtil.cpp
        src/Scene.cpp
        src/InteractiveCamera.cpp)

add_executable(PTGPU ${SOURCE_FILES})
target_link_libraries(PTGPU ${LIBRARIES})
target_include_directories(PTGPU PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/ext/imgui/include)
