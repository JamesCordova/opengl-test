cmake_minimum_required(VERSION 3.28)

project(stb_image)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} PRIVATE src/stb_image.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ./include/)