/**
 * @file shaders.c
 * @brief Implementation file for shader stuff (not pipelines)
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/
#include <SDL3/SDL.h>
#include <fileio.h>
#include <graphics.h>

bool Graphics_LoadShaderFromMem(Shader *shader,
								uint8_t *buffer, size_t size,
								const char *entrypoint,
								ShaderStage stage,
								Uint32 samplerCount,
								Uint32 uniformBufferCount,
								Uint32 storageBufferCount,
								Uint32 storageTextureCount)
{
	if(shader == NULL || buffer == NULL || size <= 0)
	{
		return false;
	}
	SDL_GPUShaderStage sdl_stage;
	switch(stage)
	{
		case SHADERSTAGE_VERTEX: sdl_stage = SDL_GPU_SHADERSTAGE_VERTEX; break;
		case SHADERSTAGE_FRAGMENT: sdl_stage = SDL_GPU_SHADERSTAGE_FRAGMENT; break;
		default: return false;
	}

	SDL_GPUShaderCreateInfo shader_info = {
		.code = buffer,
		.code_size = size,
		.entrypoint = entrypoint,
		.format = SDL_GPU_SHADERFORMAT_SPIRV, //the only one accepted for now
		.stage = sdl_stage,
		.num_samplers = samplerCount,
		.num_uniform_buffers = uniformBufferCount,
		.num_storage_buffers = storageBufferCount,
		.num_storage_textures = storageTextureCount
	};
	shader->shader = SDL_CreateGPUShader(context.device, &shader_info);
	if(shader->shader == NULL)
	{
		return false;
	}
	return true;
}

bool Graphics_LoadShaderFromFS(Shader *shader,
								const char *path,
								const char *entrypoint,
								ShaderStage stage,
								Uint32 samplerCount,
								Uint32 uniformBufferCount,
								Uint32 storageBufferCount,
								Uint32 storageTextureCount)
{
	if(shader == NULL || path == NULL) //TODO compare with incompatible strings
	{
		return false;
	}
	bool result = false;
	size_t filesize;
	Uint8 *file = FileIOReadBytes(path, &filesize);

	result = Graphics_LoadShaderFromMem(shader, file, filesize, entrypoint, stage,
										samplerCount, uniformBufferCount, storageBufferCount, storageTextureCount);
	return result;
}