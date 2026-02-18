add_library(v3d_core STATIC
    version_control.cpp
    memory_pool.cpp
    object_manager.cpp
    event_system.cpp
    logger.cpp
)

target_include_directories(v3d_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(v3d_core PUBLIC
    Threads::Threads
)

find_package(nlohmann_json 3.11.0)
if(nlohmann_json_FOUND)
    target_link_libraries(v3d_core PUBLIC
        nlohmann_json::nlohmann_json
    )
endif()

if(WIN32)
    target_link_libraries(v3d_core PUBLIC
        winmm
    )
endif()

target_compile_features(v3d_core PUBLIC cxx_std_20)
