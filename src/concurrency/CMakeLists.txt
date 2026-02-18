add_library(v3d_concurrency STATIC
    thread_pool.cpp
    task_scheduler.cpp
    async_executor.cpp
    parallel_for.cpp
)

target_include_directories(v3d_concurrency PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(v3d_concurrency PUBLIC
    Threads::Threads
    v3d_core
)

target_compile_features(v3d_concurrency PUBLIC cxx_std_20)

if(USE_CUDA)
    find_package(CUDA REQUIRED)
    target_sources(v3d_concurrency PRIVATE
        cuda_executor.cpp
    )
    target_link_libraries(v3d_concurrency PUBLIC
        CUDA::cudart
    )
endif()