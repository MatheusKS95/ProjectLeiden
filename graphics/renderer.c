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

void Graphics_CreateRenderer(Renderer *renderer, Color clear_color)
{
	if(renderer == NULL)
	{
		return;
	}
	renderer->clear_color = clear_color;
	renderer->texture_depth = SDL_CreateGPUTexture(
		context.device,
		&(SDL_GPUTextureCreateInfo) {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.width = context.width,
			.height = context.height,
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
			.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET
		}
	);
}

void Graphics_BeginDrawing(Renderer *renderer)
{
	if(renderer == NULL)
	{
		return;
	}
	renderer->cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	if (renderer->cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: AcquireGPUCommandBuffer failed: %s", SDL_GetError());
		return;
	}

	if (!SDL_WaitAndAcquireGPUSwapchainTexture(renderer->cmdbuf, context.window, &renderer->swapchain_texture, NULL, NULL))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());;
	}

	if(renderer->swapchain_texture == NULL)
	{
		return;
	}

	SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
	colorTargetInfo.texture = renderer->swapchain_texture;
	colorTargetInfo.clear_color = (SDL_FColor){ renderer->clear_color.r, renderer->clear_color.g, renderer->clear_color.b, renderer->clear_color.a };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = { 0 };
	depthStencilTargetInfo.texture = renderer->texture_depth;
	depthStencilTargetInfo.cycle = true;
	depthStencilTargetInfo.clear_depth = 1;
	depthStencilTargetInfo.clear_stencil = 0;
	depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
	depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

	renderer->render_pass = SDL_BeginGPURenderPass(renderer->cmdbuf, &colorTargetInfo, 1, &depthStencilTargetInfo);
}

void Graphics_EndDrawing(Renderer *renderer)
{
	if(renderer == NULL)
	{
		return;
	}
	if(renderer->swapchain_texture != NULL)
	{
		SDL_EndGPURenderPass(renderer->render_pass);
	}
	SDL_SubmitGPUCommandBuffer(renderer->cmdbuf);
}

//TODO
//instead of drawmodel, what about drawmesh?
//less friendly, but easier to use custom buffers, uniforms, samplers and like

/*struct studyrendering
{
	Mesh *mesh;
	Renderer *renderer;
	Pipeline pipeline;
	Sampler *sampler;
	void *vertex_ubo;
	size_t vubo_size;
	void *fragment_ubo;
	size_t fubo_size;
	also any kind of storage buffers if needed
};*/

void Graphics_DrawModelT1(Model *model, Renderer *renderer,
							Pipeline pipeline, Matrix4x4 mvp,
							Sampler *sampler)
{
	if(model == NULL || renderer == NULL)
	{
		return;
	}

	for(int i = 0; i < model->meshes.count; ++i)
	{
		SDL_BindGPUGraphicsPipeline(renderer->render_pass, pipeline);
		SDL_BindGPUVertexBuffers(renderer->render_pass, 0, &(SDL_GPUBufferBinding){ model->meshes.meshes[i].vbuffer, 0 }, 1);
		SDL_BindGPUIndexBuffer(renderer->render_pass, &(SDL_GPUBufferBinding){ model->meshes.meshes[i].ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 0, &(SDL_GPUTextureSamplerBinding){ model->meshes.meshes[i].diffuse_map->texture, sampler }, 1);
		SDL_PushGPUVertexUniformData(renderer->cmdbuf, 0, &mvp, sizeof(mvp));
		SDL_DrawGPUIndexedPrimitives(renderer->render_pass, model->meshes.meshes[i].indices.count, 1, 0, 0, 0);
	}
}

void Graphics_DrawMesh(Mesh *mesh, Renderer *renderer,
						RenderingStageDesc *desc)
{
	if(mesh == NULL || renderer == NULL || desc == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Mesh %s failed to render.", mesh->meshname);
		return;
	}

	//binding graphics pipeline
	SDL_BindGPUGraphicsPipeline(renderer->render_pass, desc->pipeline);

	//binding vertex and index buffers
	SDL_BindGPUVertexBuffers(renderer->render_pass, 0, &(SDL_GPUBufferBinding){ mesh->vbuffer, 0 }, 1);
	SDL_BindGPUIndexBuffer(renderer->render_pass, &(SDL_GPUBufferBinding){ mesh->ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);

	//bind vert storages
	if(desc->vert_storage_buffers != NULL && desc->vert_storage_buffers_count > 0)
		SDL_BindGPUVertexStorageBuffers(renderer->render_pass, 0, desc->vert_storage_buffers, desc->vert_storage_buffers_count);
	if(desc->vert_storage_textures == NULL && desc->vert_storage_textures_count > 0)
		SDL_BindGPUVertexStorageTextures(renderer->render_pass, 0, desc->vert_storage_textures, desc->vert_storage_textures_count);
	//bind frag storages
	if(desc->frag_storage_buffers != NULL && desc->frag_storage_buffers_count > 0)
		SDL_BindGPUFragmentStorageBuffers(renderer->render_pass, 0, desc->frag_storage_buffers, desc->frag_storage_buffers_count);
	if(desc->frag_storage_textures == NULL && desc->frag_storage_textures_count > 0)
		SDL_BindGPUFragmentStorageTextures(renderer->render_pass, 0, desc->frag_storage_textures, desc->frag_storage_textures_count);

	//texture samplers
	//diffuse
	if(desc->diffuse_map_or != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 0, &(SDL_GPUTextureSamplerBinding){ desc->diffuse_map_or->texture, desc->sampler }, 1);
	else if(mesh->diffuse_map != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 0, &(SDL_GPUTextureSamplerBinding){ mesh->diffuse_map->texture, desc->sampler }, 1);
	//normal
	if(desc->normal_map_or != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 1, &(SDL_GPUTextureSamplerBinding){ desc->normal_map_or->texture, desc->sampler }, 1);
	else if(mesh->normal_map != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 1, &(SDL_GPUTextureSamplerBinding){ mesh->normal_map->texture, desc->sampler }, 1);
	//specular
	if(desc->specular_map_or != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 2, &(SDL_GPUTextureSamplerBinding){ desc->specular_map_or->texture, desc->sampler }, 1);
	else if(mesh->specular_map != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 2, &(SDL_GPUTextureSamplerBinding){ mesh->specular_map->texture, desc->sampler }, 1);
	//emission
	if(desc->emission_map_or != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 3, &(SDL_GPUTextureSamplerBinding){ desc->emission_map_or->texture, desc->sampler }, 1);
	else if(mesh->emission_map != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 3, &(SDL_GPUTextureSamplerBinding){ mesh->emission_map->texture, desc->sampler }, 1);
	//height
	if(desc->height_map_or != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 4, &(SDL_GPUTextureSamplerBinding){ desc->height_map_or->texture, desc->sampler }, 1);
	else if(mesh->height_map != NULL)
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 4, &(SDL_GPUTextureSamplerBinding){ mesh->height_map->texture, desc->sampler }, 1);

	//push ubos
	if(desc->vertex_ubo_size > 0 && desc->vertex_ubo != NULL)
		SDL_PushGPUVertexUniformData(renderer->cmdbuf, 0, desc->vertex_ubo, desc->vertex_ubo_size);
	if(desc->fragment_ubo_size > 0 && desc->fragment_ubo != NULL)
		SDL_PushGPUFragmentUniformData(renderer->cmdbuf, 0, desc->fragment_ubo, desc->fragment_ubo_size);

	//finally
	SDL_DrawGPUIndexedPrimitives(renderer->render_pass, mesh->indices.count, 1, 0, 0, 0);
}

void Graphics_DrawModel(Model *model, Renderer *renderer,
						RenderingStageDesc *desc)
{
	if(model == NULL || renderer == NULL || desc == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't render model.");
		return;
	}

	for(size_t i = 0; i < model->meshes.count; i++)
	{
		Graphics_DrawMesh(&model->meshes.meshes[i], renderer, desc);
	}
}