find_package(glm CONFIG REQUIRED)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

add_library(v3d_animation STATIC
    skeleton.cpp
    bone.cpp
    animation_clip.cpp
    animation_state_machine.cpp
    animation_controller.cpp
    keyframe.cpp
    curve.cpp
    ik_solver.cpp
    pose.cpp
)

target_include_directories(v3d_animation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(v3d_animation PUBLIC
    v3d_core
    v3d_concurrency
    glm::glm
)

target_compile_features(v3d_animation PUBLIC cxx_std_20)