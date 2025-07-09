cmake_minimum_required(VERSION 3.20)

#set the correct place for built libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

#prevent installation on system directories
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

project(
	ProjectLeiden
	VERSION 0.0.1
	LANGUAGES C
)


if(MSVC)
	if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
		add_definitions(/MP) #parallelize each target, unless Ninja is the generator
	endif()
endif()

set(EXECUTABLE_NAME ${PROJECT_NAME})
add_executable(${EXECUTABLE_NAME})

#main
target_sources(${EXECUTABLE_NAME} 
PRIVATE 
	src/main.c
)

#data
target_include_directories(${EXECUTABLE_NAME} PUBLIC src/data)
target_sources(${EXECUTABLE_NAME} 
PRIVATE 
	src/data/hashtable.c
)

#filesystem
target_include_directories(${EXECUTABLE_NAME} PUBLIC src/filesystem)
target_sources(${EXECUTABLE_NAME} 
PRIVATE 
	src/filesystem/fileio.c
	src/filesystem/ini.c
)

#linmath
target_include_directories(${EXECUTABLE_NAME} PUBLIC src/linmath)
target_sources(${EXECUTABLE_NAME} 
PRIVATE 
	src/linmath/linmath.c
)

#physics
target_include_directories(${EXECUTABLE_NAME} PUBLIC src/physics)
target_sources(${EXECUTABLE_NAME}
PRIVATE
	src/physics/physics.c
)

#assets
target_include_directories(${EXECUTABLE_NAME} PUBLIC src/assets)
target_sources(${EXECUTABLE_NAME}
PRIVATE
	src/assets/camera.c
	src/assets/texture.c
	src/assets/model.c
)

#shaders
target_include_directories(${EXECUTABLE_NAME} PUBLIC src/shaders)
target_sources(${EXECUTABLE_NAME}
PRIVATE
	src/shaders/shader.c
)

#screens
target_include_directories(${EXECUTABLE_NAME} PUBLIC src/screens)
target_sources(${EXECUTABLE_NAME}
PRIVATE
	src/screens/controls.c
	src/screens/helpers.c
	src/screens/splash.c
	src/screens/test1.c
	src/screens/test2.c
)

#SDL
add_subdirectory(SDL EXCLUDE_FROM_ALL)

#SDL_image
set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_TIF OFF)
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_JPEG OFF)
set(SDLIMAGE_WEBP OFF)
add_subdirectory(SDL_image EXCLUDE_FROM_ALL)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC 
	SDL3_image::SDL3_image
	SDL3::SDL3
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

