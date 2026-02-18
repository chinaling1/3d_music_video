find_package(glm CONFIG REQUIRED)

add_library(v3d_audio STATIC
    audio_engine.cpp
    audio_buffer.cpp
    audio_source.cpp
    audio_mixer.cpp
    audio_effect.cpp
    vocal_synthesizer.cpp
    waveform_editor.cpp
    midi_processor.cpp
)

target_include_directories(v3d_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(v3d_audio PUBLIC
    v3d_core
    v3d_concurrency
    glm::glm
)

target_compile_features(v3d_audio PUBLIC cxx_std_20)

if(WIN32)
    target_link_libraries(v3d_audio PUBLIC
        winmm
    )
endif()