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

add_subdirectory(imgui-cmake) # Needs building

add_subdirectory(glad)

add_subdirectory(stb_image)
