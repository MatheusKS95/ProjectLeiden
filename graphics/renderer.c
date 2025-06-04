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
 * @file shaders.c
 * @brief Implementation file for rendering stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/
#include <SDL3/SDL.h>
#include <graphics.h>

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

CommandBuffer *Graphics_SetupCommandBuffer()
{
	return SDL_AcquireGPUCommandBuffer(context.device);
}

void Graphics_CommitCommandBuffer(CommandBuffer *cmdbuf)
{
	if(cmdbuf == NULL) return; //TODO error message
	SDL_SubmitGPUCommandBuffer(cmdbuf);
}

GPUTexture *Graphics_AcquireSwapchainTexture(CommandBuffer *cmdbuf)
{
	GPUTexture *texture;
	if(!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.window, &texture, NULL, NULL))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: Failed to acquire swapchain texture: %s", SDL_GetError());
		return NULL;
	}
	return texture;
}

RenderPass *Graphics_BeginRenderPass(CommandBuffer *cmdbuf,
										GPUTexture *render_texture,
										GPUTexture *depth_texture,
										Color clear_color)
{
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid command buffer");
		return NULL;
	}
	if(render_texture == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid render texture - please provide a texture to render (or swapchain texture)");
		return NULL;
	}

	SDL_GPUColorTargetInfo colortargetinfo = { 0 };
	colortargetinfo.texture = render_texture;
	colortargetinfo.clear_color = (SDL_FColor){ clear_color.r, clear_color.g, clear_color.b, clear_color.a };
	colortargetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colortargetinfo.store_op = SDL_GPU_STOREOP_STORE;

	if(depth_texture != NULL)
	{
		SDL_GPUDepthStencilTargetInfo depthstenciltargetinfo = { 0 };
		depthstenciltargetinfo.texture = depth_texture;
		depthstenciltargetinfo.cycle = true;
		depthstenciltargetinfo.clear_depth = 1;
		depthstenciltargetinfo.clear_stencil = 0;
		depthstenciltargetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
		depthstenciltargetinfo.store_op = SDL_GPU_STOREOP_STORE;
		depthstenciltargetinfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
		depthstenciltargetinfo.stencil_store_op = SDL_GPU_STOREOP_STORE;
		return SDL_BeginGPURenderPass(cmdbuf, &colortargetinfo, 1, &depthstenciltargetinfo);
	}
	else
	{
		return SDL_BeginGPURenderPass(cmdbuf, &colortargetinfo, 1, NULL);
	}
}

void Graphics_EndRenderPass(RenderPass *renderpass)
{
	SDL_EndGPURenderPass(renderpass);
}
