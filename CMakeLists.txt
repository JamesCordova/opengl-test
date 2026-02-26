cmake_minimum_required(VERSION 3.28)
project(opengl-test LANGUAGES CXX)

add_subdirectory(thirdparty)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/learnopengl/shader.cpp
    )
target_include_directories(${PROJECT_NAME} PRIVATE ./include/)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw GLAD stb_image)
