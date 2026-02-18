find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

add_library(v3d_io STATIC
    file_loader.cpp
    file_saver.cpp
    serializer.cpp
    format_converter.cpp
    asset_manager.cpp
)

target_include_directories(v3d_io PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(v3d_io PUBLIC
    v3d_core
    v3d_animation
    v3d_modeling
    v3d_audio
    glm::glm
    assimp::assimp
    nlohmann_json::nlohmann_json
)

target_compile_features(v3d_io PUBLIC cxx_std_20)