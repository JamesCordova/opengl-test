cmake_minimum_required(VERSION 3.28)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY  https://github.com/glfw/glfw.git
    GIT_TAG         7b6aead9fb88b3623e3b3725ebb42670cbe4c579 #refs/tags/3.4
    GIT_SHALLOW     TRUE
    SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/glfw
)

FetchContent_MakeAvailable(glfw)


FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         8d1fd52e5ab5590e2c81768ace50c72bae28f2ed #refs/tags/1.0.3
    GIT_SHALLOW     TRUE
    SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY  https://github.com/ocornut/imgui.git
    GIT_TAG         2a1b69f05748ad909f03acf4533447cac1331611 #refs/tags/v1.92.6-docking
    GIT_SHALLOW     TRUE
    SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/imgui-cmake/imgui
)

FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY  https://github.com/assimp/assimp.git
    GIT_TAG         e0b52347c6e52de2827ec957a9ebf00ce3c54f79 #refs/tags/v6.0.4
    GIT_SHALLOW     TRUE
    SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/assimp
)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE) # It reduce targets, it's recommended
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE) # As submodule, doesn't change compilation time 

if (MSVC)
    set(ASSIMP_BUILD_USE_CCACHE OFF CACHE BOOL "" FORCE) # default to ON: conflict with rc
endif()
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY  https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG         0a0221a1347e2f1e07c395263540026e9a0aa7c7 #refs/tags/VER-2-14-3
    GIT_SHALLOW     TRUE
    SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/freetype
)

FetchContent_MakeAvailable(freetype)

add_subdirectory(imgui-cmake) # Needs building

add_subdirectory(glad)

add_subdirectory(stb_image)
