cmake_minimum_required(VERSION 3.30)
project(re_minecraft VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 26)

add_compile_options(-Wall -Wextra -Wpedantic)

set(
        SOURCES

        lib/glad/src/glad.c
        lib/stbi/stb_image.cpp
        lib/imgui/src/imgui.cpp
        lib/imgui/src/imgui_draw.cpp
        lib/imgui/src/imgui_impl_opengl3.cpp
        lib/imgui/src/imgui_impl_glfw.cpp
        lib/imgui/src/imgui_tables.cpp
        lib/imgui/src/imgui_widgets.cpp

        main.cpp
        src/Engine/Engine.cpp
        src/Engine/Camera/Camera.cpp
        src/Engine/Render/VAO/VAO.cpp
        src/Engine/Render/VBO/VBO.cpp
        src/Engine/Render/Shader/Shader.cpp
        src/Engine/Render/Atlas/Atlas.cpp
        src/Engine/Render/TextureManager/TextureManager.cpp
        src/Content/ChunkMesh/ChunkMesh.cpp
        src/Content/ChunkMeshManager/ChunkMeshManager.cpp
        src/Content/Chunk/Chunk.cpp
        src/Content/World/World.cpp
        src/Content/BlockRegistry/BlockRegistry.cpp
        src/Content/GUI/GUI.cpp
)

set(
        HEADERS
        ${CMAKE_SOURCE_DIR}/lib
        ${CMAKE_SOURCE_DIR}/lib/imgui
        ${CMAKE_SOURCE_DIR}/lib/fastnoiselite
        ${CMAKE_SOURCE_DIR}/src/Engine
        ${CMAKE_SOURCE_DIR}/src/Engine/Camera
        ${CMAKE_SOURCE_DIR}/src/Engine/Input
        ${CMAKE_SOURCE_DIR}/src/Engine/Raycast
        ${CMAKE_SOURCE_DIR}/src/Engine/Render/VAO
        ${CMAKE_SOURCE_DIR}/src/Engine/Render/VBO
        ${CMAKE_SOURCE_DIR}/src/Engine/Render/Shader
        ${CMAKE_SOURCE_DIR}/src/Engine/Render/Atlas
        ${CMAKE_SOURCE_DIR}/src/Engine/Render/TextureManager
        ${CMAKE_SOURCE_DIR}/src/Content/ChunkMesh
        ${CMAKE_SOURCE_DIR}/src/Content/ChunkMeshManager
        ${CMAKE_SOURCE_DIR}/src/Content/Chunk
        ${CMAKE_SOURCE_DIR}/src/Content/World
        ${CMAKE_SOURCE_DIR}/src/Content/BlockRegistry
        ${CMAKE_SOURCE_DIR}/src/Content/GUI
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
