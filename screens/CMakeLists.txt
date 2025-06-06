add_library(screens STATIC
	simple.c
	controls.c
	splash.c
	demopostproc.c
)
target_include_directories(screens PUBLIC .)
target_link_libraries(screens PUBLIC SDL3_image::SDL3_image)
target_link_libraries(screens PUBLIC SDL3::SDL3)
target_link_libraries(screens PRIVATE linmath)
target_link_libraries(screens PRIVATE filesystem)
target_link_libraries(screens PRIVATE data)
target_link_libraries(screens PRIVATE input)
target_link_libraries(screens PRIVATE graphics)
target_link_libraries(screens PRIVATE audio)
target_link_libraries(screens PRIVATE leiden)
