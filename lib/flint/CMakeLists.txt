###############
#
#      @
#       @
#     @@@
#
###############

cmake_minimum_required(
    VERSION 3.20
)

set(
    CMAKE_CXX_STANDARD 20
)

set(
    CMAKE_CXX_ABI_COMPILED ON
)

#set(
#    CMAKE_CXX_FLAGS -rdynamic
#)

add_compile_options(
    -Wall -Wextra -Werror -Wno-error=vla
)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    ${PROJECT_SOURCE_DIR}/lib${PROJECT_NAME}.a
)

file(
    GLOB_RECURSE
    CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(flint STATIC
    ${CPP_FILES}
)

target_include_directories(flint PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)
