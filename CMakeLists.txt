cmake_minimum_required(VERSION 3.30)
project(re_minecraft VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 26)

add_compile_options(-Wall -Wextra -Wpedantic)

set(
        SOURCES
        lib/glad/src/glad.c
        lib/stbi/stb_image.cpp
        main.cpp
        src/Engine/Engine.cpp
        src/Engine/Camera/Camera.cpp
        src/Engine/Drawer/VAO/VAO.cpp
        src/Engine/Drawer/CubeVAO/CubeVAO.cpp
        src/Engine/Drawer/VBO/VBO.cpp
        src/Engine/Drawer/EBO/EBO.cpp
        src/Engine/Drawer/Shader/Shader.cpp
        src/Engine/Drawer/Atlas/Atlas.cpp
        src/Content/Chunk/Chunk.cpp
        src/Content/Map/World.cpp
)

set(
        HEADERS
        ${CMAKE_SOURCE_DIR}/lib
        ${CMAKE_SOURCE_DIR}/src/Engine
        ${CMAKE_SOURCE_DIR}/src/Engine/Camera
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/VAO
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/CubeVAO
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/VBO
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/EBO
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/Shader
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/Atlas
        ${CMAKE_SOURCE_DIR}/src/Content/Chunk
        ${CMAKE_SOURCE_DIR}/src/Content/Map
)

add_executable(
        re_minecraft
        ${SOURCES}
)


target_include_directories(
        re_minecraft
        PRIVATE
        ${HEADERS}
)

target_link_libraries(
        re_minecraft
        PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/GLFW/libglfw3.a
)
