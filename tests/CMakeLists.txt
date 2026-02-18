cmake_minimum_required(VERSION 3.20)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

add_executable(v3d_tests
    test_main.cpp
    utils/test_utils.cpp
    utils/test_data_generator.cpp
    utils/test_reporter.cpp
    core/test_version_control.cpp
    core/test_memory_pool.cpp
    core/test_object_manager.cpp
    core/test_event_system.cpp
    core/test_logger.cpp
    concurrency/test_thread_pool.cpp
    concurrency/test_task_scheduler.cpp
    concurrency/test_async_executor.cpp
    animation/test_skeleton.cpp
    animation/test_pose.cpp
    animation/test_curve.cpp
    animation/test_ik_solver.cpp
    animation/test_animation_clip.cpp
    modeling/test_mesh.cpp
    modeling/test_vertex.cpp
    modeling/test_face.cpp
    modeling/test_edge.cpp
    modeling/test_mesh_operations.cpp
    modeling/test_subdivision.cpp
    modeling/test_material.cpp
    audio/test_audio_buffer.cpp
    audio/test_audio_engine.cpp
    audio/test_audio_effect.cpp
    audio/test_vocal_synthesizer.cpp
    io/test_file_loader.cpp
    io/test_file_saver.cpp
    io/test_serializer.cpp
    io/test_format_converter.cpp
    io/test_asset_manager.cpp
)

target_include_directories(v3d_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(v3d_tests PRIVATE
    v3d_core
    v3d_animation
    v3d_modeling
    v3d_audio
    v3d_io
    v3d_concurrency
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    Threads::Threads
    glm::glm
    assimp::assimp
)

target_compile_features(v3d_tests PRIVATE cxx_std_20)

enable_testing()
include(GoogleTest)
gtest_discover_tests(v3d_tests)

add_custom_target(test_coverage
    COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1 $<TARGET_FILE:v3d_tests> --gtest_output=xml:test_results.xml
    COMMAND lcov --capture --directory ${CMAKE_BINARY_DIR} --output-file coverage.info
    COMMAND lcov --remove coverage.info '/usr/*' '*/tests/*' '*/gtest/*' --output-file coverage.info
    COMMAND genhtml coverage.info --output-directory coverage_html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating test coverage report"
)
