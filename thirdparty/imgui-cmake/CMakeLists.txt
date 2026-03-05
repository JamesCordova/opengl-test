cmake_minimum_required(VERSION 3.28)

project(imgui)

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

add_library(${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    "${IMGUI_DIR}/misc/cpp"
)

target_sources(${PROJECT_NAME} PRIVATE
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
    "${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp"
)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw)