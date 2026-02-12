cmake_minimum_required(VERSION 3.30)
project(farfield VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 26)

add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_CXX_ABI_COMPILED ON)

file(
    GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

#Local libs
list(APPEND SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/src/glad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/stbi/stb_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/src/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/src/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/src/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/src/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/src/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/src/imgui_widgets.cpp
)

#Headers
set(
    HEADERS
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/json
    ${CMAKE_SOURCE_DIR}/lib/imgui
    ${CMAKE_SOURCE_DIR}/lib/fastnoiselite

    ${CMAKE_SOURCE_DIR}/src/Engine
    ${CMAKE_SOURCE_DIR}/src/Engine/Input
    ${CMAKE_SOURCE_DIR}/src/Engine/Raycast
    ${CMAKE_SOURCE_DIR}/src/Engine/Render/Frustum
    ${CMAKE_SOURCE_DIR}/src/Engine/Render/Shader
    ${CMAKE_SOURCE_DIR}/src/Engine/Render/VAO
    ${CMAKE_SOURCE_DIR}/src/Engine/Render/VBO
    ${CMAKE_SOURCE_DIR}/src/Engine/Render/Viewport
    ${CMAKE_SOURCE_DIR}/src/Engine/Render/Font
    ${CMAKE_SOURCE_DIR}/src/Engine/Settings
    ${CMAKE_SOURCE_DIR}/src/Engine/ThreadPool
    ${CMAKE_SOURCE_DIR}/src/Engine/Utils

    ${CMAKE_SOURCE_DIR}/src/Content/
    ${CMAKE_SOURCE_DIR}/src/Content/Chunks/Chunk
    ${CMAKE_SOURCE_DIR}/src/Content/Chunks/ChunkManager
    ${CMAKE_SOURCE_DIR}/src/Content/Chunks/ChunkMesh
    ${CMAKE_SOURCE_DIR}/src/Content/Chunks/ChunkMeshManager
    ${CMAKE_SOURCE_DIR}/src/Content/GUI
    ${CMAKE_SOURCE_DIR}/src/Content/Meshes
    ${CMAKE_SOURCE_DIR}/src/Content/Meshes/EntityMeshData
    ${CMAKE_SOURCE_DIR}/src/Content/NeighborAccess
    ${CMAKE_SOURCE_DIR}/src/Content/PlayerController
    ${CMAKE_SOURCE_DIR}/src/Content/TerrainGenerator
    ${CMAKE_SOURCE_DIR}/src/Content/World
    ${CMAKE_SOURCE_DIR}/src/Content/Registries/BlockRegistry
    ${CMAKE_SOURCE_DIR}/src/Content/Registries/TextureRegistry
    ${CMAKE_SOURCE_DIR}/src/Content/Registries/PrefabRegistry
    ${CMAKE_SOURCE_DIR}/src/Content/Registries/MeshRegistry
)

#add_subdirectory(lib/flint) # Comment for now as the lib won't compile on Windows
add_subdirectory(lib/GLFW)

add_executable(farfield ${SOURCES})

#target_link_libraries(farfield PRIVATE flint) # Comment for now as the lib won't compile on Windows
target_link_libraries(farfield PRIVATE glfw)

target_include_directories(farfield PRIVATE ${HEADERS})