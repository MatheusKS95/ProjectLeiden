/*
 * Copyright (C) 2025 Matheus Klein Schaefer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <fileio.h>
#include <shader.h>

SDL_GPUShader* LoadShader(const char *path,
							SDL_GPUDevice *device,
							SDL_GPUShaderStage stage,
							Uint32 samplerCount,
							Uint32 uniformBufferCount,
							Uint32 storageBufferCount,
							Uint32 storageTextureCount)
{
	if(path == NULL || SDL_strcmp(path, "") == 0)
	{
		return NULL;
	}
	size_t filesize;
	Uint8 *file = FileIOReadBytes(path, &filesize);

	if(file == NULL)
	{
		//error message
		return NULL;
	}

	SDL_GPUShaderCreateInfo shader_info = {
		.code = file,
		.code_size = filesize,
		.entrypoint = "main",
		.format = SDL_GPU_SHADERFORMAT_SPIRV, //the only one accepted for now
		.stage = stage,
		.num_samplers = samplerCount,
		.num_uniform_buffers = uniformBufferCount,
		.num_storage_buffers = storageBufferCount,
		.num_storage_textures = storageTextureCount
	};
	return SDL_CreateGPUShader(device, &shader_info);
}
