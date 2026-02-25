cmake_minimum_required(VERSION 3.28)

include(ExternalProject)

ExternalProject_Add(
    GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)