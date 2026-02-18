cmake_minimum_required(VERSION 3.20)

project(v3d_studio VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(v3d_studio WIN32
    main.cpp
    ui/main_window.cpp
    ui/gl_renderer.cpp
    panels/panel_3d_view.cpp
    panels/panel_timeline.cpp
    panels/panel_properties.cpp
    panels/panel_asset_browser.cpp
    panels/panel_modeling_tools.cpp
    panels/panel_ai_assistant.cpp
    core/project_manager.cpp
    core/audio_converter.cpp
    modeling/geometry_factory.cpp
)

target_include_directories(v3d_studio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(v3d_studio PRIVATE
    v3d_core
    v3d_animation
    v3d_modeling
    v3d_audio
    v3d_io
    v3d_concurrency
    glm::glm
    nlohmann_json::nlohmann_json
    comctl32
    shell32
    opengl32
    glu32
)

target_compile_definitions(v3d_studio PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

set_target_properties(v3d_studio PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)

if(MSVC)
    target_compile_options(v3d_studio PRIVATE /W4 /utf-8)
endif()
