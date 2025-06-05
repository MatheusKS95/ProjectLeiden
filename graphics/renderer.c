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
	if(renderpass == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid render pass.");
		return;
	}
	SDL_EndGPURenderPass(renderpass);
}

void Graphics_BindPipeline(RenderPass *renderpass,
							Pipeline *pipeline)
{
	if(renderpass == NULL || pipeline == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid render pass and/or invalid pipeline.");
		return;
	}
	SDL_BindGPUGraphicsPipeline(renderpass, pipeline);
}

void Graphics_BindVertexBuffers(RenderPass *renderpass,
								GPUBuffer *buffer,
								Uint32 buffer_offset,
								Uint32 first_slot,
								Uint32 num_bindings)
{
	if(renderpass == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid render pass.");
		return;
	}
	SDL_GPUBufferBinding binding;
	binding.buffer = buffer;
	binding.offset = buffer_offset;
	SDL_BindGPUVertexBuffers(renderpass, first_slot, &binding, num_bindings);
}

void Graphics_BindIndexBuffers(RenderPass *renderpass,
								GPUBuffer *buffer,
								Uint32 buffer_offset)
{
	if(renderpass == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid render pass.");
		return;
	}
	SDL_GPUBufferBinding binding;
	binding.buffer = buffer;
	binding.offset = buffer_offset;
	SDL_BindGPUIndexBuffer(renderpass, &binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
}

void Graphics_BindMeshBuffers(RenderPass *renderpass, Mesh *mesh)
{
	if(renderpass == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid render pass.");
		return;
	}
	if(mesh == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid mesh.");
		return;
	}

	Graphics_BindVertexBuffers(renderpass, mesh->vbuffer, 0, 0, 1);
	Graphics_BindIndexBuffers(renderpass, mesh->ibuffer, 0);
}

void Graphics_BindFragmentSampledTexture(RenderPass *renderpass,
											Texture2D *texture,
											Sampler *sampler,
											Uint32 first_slot,
											Uint32 num_bindings)
{
	if(renderpass == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid render pass.");
		return;
	}
	if(texture == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid texture.");
		return;
	}
	if(sampler == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid sampler.");
		return;
	}
	SDL_GPUTextureSamplerBinding binding;
	binding.sampler = sampler;
	binding.texture = texture->texture;
	SDL_BindGPUFragmentSamplers(renderpass, first_slot, &binding, num_bindings);
}

void Graphics_BindFragmentSampledGPUTexture(RenderPass *renderpass,
											GPUTexture *texture,
											Sampler *sampler,
											Uint32 first_slot,
											Uint32 num_bindings)
{
	if(renderpass == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid render pass.");
		return;
	}
	if(texture == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid texture.");
		return;
	}
	if(sampler == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid sampler.");
		return;
	}
	SDL_GPUTextureSamplerBinding binding;
	binding.sampler = sampler;
	binding.texture = texture;
	SDL_BindGPUFragmentSamplers(renderpass, first_slot, &binding, num_bindings);
}

void Graphics_PushVertexUniforms(CommandBuffer *cmdbuf,
									Uint32 slot, void *data,
									size_t length)
{
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid command buffer");
		return;
	}
	if(data == NULL || length <= 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid uniform data buffer");
		return;
	}
	SDL_PushGPUVertexUniformData(cmdbuf, slot, data, length);
}

void Graphics_PushFragmentUniforms(CommandBuffer *cmdbuf,
									Uint32 slot, void *data,
									size_t length)
{
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid command buffer");
		return;
	}
	if(data == NULL || length <= 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid uniform data buffer");
		return;
	}
	SDL_PushGPUFragmentUniformData(cmdbuf, slot, data, length);
}

void Graphics_DrawPrimitives(RenderPass *renderpass,
								size_t num_indices,
								Uint32 num_instances,
								int first_index, int vertex_offset,
								Uint32 first_instance)
{
	if(renderpass == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: invalid render pass.");
		return;
	}
	SDL_DrawGPUIndexedPrimitives(renderpass, num_indices, num_instances, first_index, vertex_offset, first_instance);
}
