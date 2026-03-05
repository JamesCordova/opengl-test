cmake_minimum_required(VERSION 3.28)
project(opengl-test LANGUAGES CXX)

add_subdirectory(thirdparty)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/learnopengl/shader.cpp
    src/learnopengl/errorReporting.cpp
    )
target_include_directories(${PROJECT_NAME} PRIVATE ./include/)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw GLAD stb_image glm imgui)

set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")

add_custom_target(sync_assets
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,create_symlink,copy_directory>
            "${ASSETS_SOURCE_DIR}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Debug: symlink assets | Other configs: copy assets"
    VERBATIM
)

add_dependencies(${PROJECT_NAME} sync_assets)