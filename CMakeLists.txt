cmake_minimum_required(VERSION 3.20)
project(3DVideoStudio VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(USE_CUDA "Enable CUDA support" OFF)
option(USE_OPTIX "Enable OptiX ray tracing" OFF)

find_package(Threads REQUIRED)

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    find_package(GMock)
endif()

add_subdirectory(src/core)
add_subdirectory(src/animation)
add_subdirectory(src/modeling)
add_subdirectory(src/audio)
add_subdirectory(src/io)
add_subdirectory(src/concurrency)
# add_subdirectory(src/ui)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    # add_subdirectory(examples)
endif()

add_subdirectory(app)