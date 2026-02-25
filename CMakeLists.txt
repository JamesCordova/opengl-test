cmake_minimum_required(VERSION 3.28)
project(opengl-test LANGUAGES CXX)

# add_subdirectory(thirdparty)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
