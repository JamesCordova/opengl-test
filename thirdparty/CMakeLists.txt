cmake_minimum_required(VERSION 3.28)

add_subdirectory(glfw)

add_subdirectory(glad)

add_subdirectory(stb_image)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
    GIT_SHALLOW     TRUE
    SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

FetchContent_MakeAvailable(glm)