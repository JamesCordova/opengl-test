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

add_subdirectory(glad)

add_subdirectory(stb_image)
