# This is only meant to be included as a subdirectory in another project.
# It assumes that targets 'SDL3' exist.

add_library(graphics STATIC
	implementations.c
	graphics.c
	textures.c
	camera.c
	lights.c
	models.c
	renderer.c
	shaders.c
)
target_compile_definitions(graphics PUBLIC USING_SDLGPU)
target_include_directories(graphics PUBLIC .)
target_link_libraries(graphics PUBLIC SDL3_image::SDL3_image)
target_link_libraries(graphics PUBLIC SDL3::SDL3)
target_link_libraries(graphics PRIVATE filesystem)
target_link_libraries(graphics PRIVATE linmath)
target_link_libraries(graphics PRIVATE data)
