# This is only meant to be included as a subdirectory in another project.
# It assumes that targets 'SDL3' exist.

# The audio-miniaudio target
add_library(audio STATIC audio.c music.c sound.c)
target_include_directories(audio PUBLIC .)
target_link_libraries(audio PUBLIC SDL3::SDL3 dl m)
target_link_libraries(audio PRIVATE filesystem)
