add_library(graphics STATIC
	implementations.c
	graphics.c
	textures.c
	camera.c
	lights.c
	models.c
	shaders.c
	skybox.c
	materials.c
)

target_include_directories(graphics PUBLIC .)
target_link_libraries(graphics PUBLIC SDL3_image::SDL3_image)
target_link_libraries(graphics PUBLIC SDL3::SDL3)
target_link_libraries(graphics PRIVATE data)
target_link_libraries(graphics PRIVATE filesystem)
target_link_libraries(graphics PRIVATE linmath)
