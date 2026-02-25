cmake_minimum_required(VERSION 3.28)

project(GLAD)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} PRIVATE src/glad.c)
target_include_directories(${PROJECT_NAME} PUBLIC include)
