find_package(glm CONFIG REQUIRED)

add_library(v3d_modeling STATIC
    mesh.cpp
    vertex.cpp
    face.cpp
    edge.cpp
    mesh_builder.cpp
    mesh_operations.cpp
    subdivision.cpp
    uv_editor.cpp
    material.cpp
)

target_include_directories(v3d_modeling PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(v3d_modeling PUBLIC
    v3d_core
    v3d_concurrency
    glm::glm
)

target_compile_features(v3d_modeling PUBLIC cxx_std_20)