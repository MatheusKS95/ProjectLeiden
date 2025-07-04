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

#include <SDL3/SDL.h>
#include <fileio.h>
#include <assets.h>
#include <shader.h>
#include <screens.h>

void SCR_CreateEffectBuffers(EffectBuffers *buffers)
{
	if(buffers == NULL)
	{
		return;
	}

	buffers->vbuffer = SDL_CreateGPUBuffer(
		drawing_context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = sizeof(EffectVertex) * 4
		}
	);

	buffers->ibuffer = SDL_CreateGPUBuffer(
		drawing_context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_INDEX,
			.size = sizeof(Uint32) * 6
		}
	);

	SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
		drawing_context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(EffectVertex) * 4) + (sizeof(Uint32) * 6)
		}
	);

	EffectVertex* transferData = SDL_MapGPUTransferBuffer(
		drawing_context.device,
		bufferTransferBuffer,
		false
	);

	transferData[0] = (EffectVertex) { -1,  1, 0, 0, 0 };
	transferData[1] = (EffectVertex) {  1,  1, 0, 1, 0 };
	transferData[2] = (EffectVertex) {  1, -1, 0, 1, 1 };
	transferData[3] = (EffectVertex) { -1, -1, 0, 0, 1 };

	Uint32* indexData = (Uint32*) &transferData[4];
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 0;
	indexData[4] = 2;
	indexData[5] = 3;

	SDL_UnmapGPUTransferBuffer(drawing_context.device, bufferTransferBuffer);

	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(drawing_context.device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = bufferTransferBuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = buffers->vbuffer,
			.offset = 0,
			.size = sizeof(EffectVertex) * 4
		},
		false
	);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = bufferTransferBuffer,
			.offset = sizeof(EffectVertex) * 4
		},
		&(SDL_GPUBufferRegion) {
			.buffer = buffers->ibuffer,
			.offset = 0,
			.size = sizeof(Uint32) * 6
		},
		false
	);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_ReleaseGPUTransferBuffer(drawing_context.device, bufferTransferBuffer);
}

void SCR_ReleaseEffectBuffers(EffectBuffers *buffers)
{
	if(buffers != NULL)
	{
		SDL_ReleaseGPUBuffer(drawing_context.device, buffers->vbuffer);
		SDL_ReleaseGPUBuffer(drawing_context.device, buffers->ibuffer);
	}
}