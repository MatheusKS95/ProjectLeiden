cmake_minimum_required(VERSION 3.5)

include(FetchContent)

project(
	ProjectLeiden
	VERSION 0.0.1
	LANGUAGES C
)

#SDL3 - steam builds should tick this off
option(ROCKPORT_SDL_VENDORED "I don't have SDL3 installed" ON)
if(ROCKPORT_SDL_VENDORED)
	#FetchContent_Declare(
	#	SDL3
	#	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	#	GIT_TAG release-3.2.6
	#)
	#FetchContent_MakeAvailable(SDL3)
	add_subdirectory(vendored/SDL)
else()
	find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

option(ROCKPORT_SDLIMAGE_VENDORED "I don't have SDL3_image installed" ON)
if(ROCKPORT_SDLIMAGE_VENDORED)
	set(SDLIMAGE_VENDORED ON)
	set(SDLIMAGE_TIF OFF)
	set(SDLIMAGE_AVIF OFF)
	set(SDLIMAGE_JPEG OFF)
	set(SDLIMAGE_WEBP OFF)
	#FetchContent_Declare(
	#	SDL3_image
	#	GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
	#	GIT_TAG release-3.2.4
	#)
	#FetchContent_MakeAvailable(SDL3_image)
	add_subdirectory(vendored/SDL_image)
else()
	find_package(SDL3_image REQUIRED CONFIG REQUIRED COMPONENTS SDL3_image)
endif()

option(ROCKPORT_ASSIMP_VENDORED "I don't have assimp installed" ON)
if(ROCKPORT_ASSIMP_VENDORED)
	set(ASSIMP_NO_EXPORT ON)
	#set(ASSIMP_BUILD_ZLIB ON)
	set(ASSIMP_BUILD_TESTS OFF)
	set(ASSIMP_INSTALL OFF)
	set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
	#FetchContent_Declare(
	#	assimp
	#	GIT_REPOSITORY https://github.com/assimp/assimp/
	#	GIT_TAG v5.4.3
	#)
	#FetchContent_MakeAvailable(assimp)
	add_subdirectory(vendored/assimp)
else()
	find_package(assimp REQUIRED CONFIG REQUIRED COMPONENTS assimp)
endif()

add_subdirectory(data)
add_subdirectory(filesystem)
add_subdirectory(graphics)
#add_subdirectory(audio-miniaudio)
add_subdirectory(input)
add_subdirectory(linmath)
add_subdirectory(demos)

add_executable(ProjectLeiden main.c)

# Adicione comandos de instalação para o executável do jogo (test)
#install(TARGETS ProjectRockport RUNTIME DESTINATION bin)

target_link_libraries(ProjectLeiden PRIVATE SDL3_image::SDL3_image SDL3::SDL3)
target_link_libraries(ProjectLeiden PRIVATE assimp::assimp)
target_link_libraries(ProjectLeiden PRIVATE data)
target_link_libraries(ProjectLeiden PRIVATE filesystem)
target_link_libraries(ProjectLeiden PRIVATE graphics)
#target_link_libraries(ProjectLeiden PRIVATE audio)
target_link_libraries(ProjectLeiden PRIVATE input)
target_link_libraries(ProjectLeiden PRIVATE linmath)
target_link_libraries(ProjectLeiden PRIVATE demos)
