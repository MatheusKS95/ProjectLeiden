# This is only meant to be included as a subdirectory in another project.
# It assumes that targets 'SDL3' exist.

# The rockport-util target
add_library(demos STATIC
	clearscreen.c
	demo1.c
	demo2.c
)
target_include_directories(demos PUBLIC .)
target_link_libraries(demos PUBLIC SDL3_image::SDL3_image)
target_link_libraries(demos PUBLIC SDL3::SDL3)
target_link_libraries(demos PRIVATE linmath)
target_link_libraries(demos PRIVATE filesystem)
target_link_libraries(demos PRIVATE data)
target_link_libraries(demos PRIVATE input)
target_link_libraries(demos PRIVATE graphics)
