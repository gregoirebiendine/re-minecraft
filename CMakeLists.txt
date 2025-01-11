cmake_minimum_required(VERSION 3.30)
project(re_minecraft VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 26)

set(
        SOURCES
        main.cpp
        src/Engine/Engine.cpp
        src/Engine/Camera/Camera.cpp
        src/Engine/Drawer/Drawer.cpp
        src/Engine/Drawer/VAO/VAO.cpp
        src/Engine/Drawer/CubeVAO/CubeVAO.cpp
        src/Engine/Drawer/VBO/VBO.cpp
        src/Engine/Drawer/EBO/EBO.cpp
        src/Engine/Drawer/Shaders/Shaders.cpp
        src/Engine/Drawer/Texture/Texture.cpp
        src/Content/Chunk/Chunk.cpp
)

set(
        LIB_SOURCES
        lib/glad/src/glad.c
        lib/stbi/src/stb_image.c
)

set(
        HEADERS
        ${CMAKE_SOURCE_DIR}/lib/
        ${CMAKE_SOURCE_DIR}/src/
        ${CMAKE_SOURCE_DIR}/src/Engine
        ${CMAKE_SOURCE_DIR}/src/Engine/Camera
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/VAO
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/CubeVAO
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/VBO
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/EBO
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/Shaders
        ${CMAKE_SOURCE_DIR}/src/Engine/Drawer/Texture
        ${CMAKE_SOURCE_DIR}/src/Content/Chunk
)

add_executable(
        re_minecraft
        ${LIB_SOURCES}
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
