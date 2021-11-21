cmake_minimum_required(VERSION 3.16)
project(GpuRay)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib/glfw)

add_executable(GpuRay
        lib/glad/src/glad.c
        lib/SimplexNoise/SimplexNoise.cpp

        src/main.cpp
        src/Loader.cpp
        src/Shader.cpp)

target_include_directories(GpuRay PRIVATE lib/glm lib/glfw/include lib/glad/include lib/imgui lib/inipp/include lib/portable-file-dialogs/include lib/SimplexNoise)
target_link_libraries(GpuRay glfw ${GLFW_LIBRARIES})
