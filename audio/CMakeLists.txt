# This is only meant to be included as a subdirectory in another project.
# It assumes that targets 'SDL3' exist.

# The audio-miniaudio target
add_library(audio STATIC audio.c music.c sound.c)
target_include_directories(audio PUBLIC .)
target_link_libraries(audio PUBLIC SDL3::SDL3 m)
if(LINUX)
	target_link_libraries(audio PUBLIC dl)
endif()
if(UNIX AND NOT APPLE) #macgyverism (ideally I should avoid pthread directly)
	target_link_libraries(audio PUBLIC pthread)
endif()
target_link_libraries(audio PRIVATE filesystem)
