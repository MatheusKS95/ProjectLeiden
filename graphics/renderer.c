//this will be removed later, each screen from a game will have their own rendering

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

/**
 * @file renderer.c
 * @brief Central implementation for rendering stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <graphics.h>
#include <SDL3/SDL.h>

bool Graphics_CreateAndUploadStorageBuffer(StorageBuffer *buffer,
									void *data, size_t size)
{
	if(buffer == NULL || data == NULL || size <= 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't generate storage buffer.");
		return false;
	}

	buffer = SDL_CreateGPUBuffer(
		context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
			.size = size
		}
	);
	SDL_GPUTransferBuffer* transferbuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = size
		}
	);
	void* buffer_transferdata = SDL_MapGPUTransferBuffer(context.device, transferbuffer, false);
	SDL_memcpy(buffer_transferdata, data, size);
	SDL_UnmapGPUTransferBuffer(context.device, transferbuffer);

	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = transferbuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = buffer,
			.offset = 0,
			.size = size
		},
		false
	);

	SDL_EndGPUCopyPass(copyPass);
	SDL_ReleaseGPUTransferBuffer(context.device, transferbuffer);
	SDL_SubmitGPUCommandBuffer(cmdbuf);

	return true;
}

void Graphics_ReleaseStorageBuffer(StorageBuffer *buffer)
{
	if(buffer == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't release invalid storage buffer.");
		return;
	}
	SDL_ReleaseGPUBuffer(context.device, buffer);
}