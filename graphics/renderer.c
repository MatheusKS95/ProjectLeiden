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

static void drawskybox(Skybox *skybox, Camera *camera, SDL_GPURenderPass *render_pass, SDL_GPUCommandBuffer *cmdbuf)
{
	if(skybox == NULL || camera == NULL || render_pass == NULL || cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't render skybox.");
		return;
	}
	Matrix4x4 cam_view = camera->view;
	cam_view.ad = cam_view.bd = cam_view.cd = cam_view.dd = 0.0f;
	cam_view.da = cam_view.db = cam_view.dc = 0.0f;
	Matrix4x4 skyboxviewproj;
	skyboxviewproj = Matrix4x4_Mul(cam_view, camera->projection);

	SDL_BindGPUGraphicsPipeline(render_pass, pipelines.skybox);
	SDL_BindGPUVertexBuffers(render_pass, 0, &(SDL_GPUBufferBinding){ skybox->vertex_buffer, 0 }, 1);
	SDL_BindGPUIndexBuffer(render_pass, &(SDL_GPUBufferBinding){ skybox->index_buffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &(SDL_GPUTextureSamplerBinding){ skybox->gputexture, skybox->sampler }, 1);
	SDL_PushGPUVertexUniformData(cmdbuf, 0, &skyboxviewproj, sizeof(skyboxviewproj));
	SDL_DrawGPUIndexedPrimitives(render_pass, 36, 1, 0, 0, 0);
}

static void drawmodelsimple(Model *model, Matrix4x4 mvp, Sampler *sampler, SDL_GPURenderPass *render_pass, SDL_GPUCommandBuffer *cmdbuf)
{
	if(model == NULL || render_pass == NULL || cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't render model.");
		return;
	}

	for(size_t i = 0; i < model->meshes.count; i++)
	{
		Mesh *mesh = &model->meshes.meshes[i];
		//binding graphics pipeline
		SDL_BindGPUGraphicsPipeline(render_pass, pipelines.simple);

		//binding vertex and index buffers
		SDL_BindGPUVertexBuffers(render_pass, 0, &(SDL_GPUBufferBinding){ mesh->vbuffer, 0 }, 1);
		SDL_BindGPUIndexBuffer(render_pass, &(SDL_GPUBufferBinding){ mesh->ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);

		//texture samplers
		Material *material = Graphics_GetMaterialByName(&model->materials, mesh->material_name);
		if(material == NULL) continue;
		Texture2D *diffuse = material->diffuse_map != NULL ? material->diffuse_map : &default_textures.default_diffuse;
		SDL_BindGPUFragmentSamplers(render_pass, 0, &(SDL_GPUTextureSamplerBinding){ diffuse->texture, sampler }, 1);

		//UBO
		SDL_PushGPUVertexUniformData(cmdbuf, 0, &mvp, sizeof(mvp));
		SDL_DrawGPUIndexedPrimitives(render_pass, mesh->indices.count, 1, 0, 0, 0);
	}
}

void Graphics_DrawSimple(SimpleRenderer *renderer,
							Color clear_color,
							Camera *camera)
{
	if(renderer == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't render anything. Make sure you send the correct structure.");
		return;
	}

	SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to acquire command buffer.");
		return;
	}

	SDL_GPUTexture *swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.window, &swapchain_texture, NULL, NULL))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to acquire swapchain texture: %s", SDL_GetError());;
	}

	if(swapchain_texture == NULL)
	{
		return;
	}

	SDL_GPUTexture *texture_depth = SDL_CreateGPUTexture(
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

	SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
	colorTargetInfo.texture = swapchain_texture;
	colorTargetInfo.clear_color = (SDL_FColor){ clear_color.r, clear_color.g, clear_color.b, clear_color.a };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = { 0 };
	depthStencilTargetInfo.texture = texture_depth;
	depthStencilTargetInfo.cycle = true;
	depthStencilTargetInfo.clear_depth = 1;
	depthStencilTargetInfo.clear_stencil = 0;
	depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
	depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthStencilTargetInfo);

	//render skybox
	if(renderer->skybox != NULL)
	{
		drawskybox(renderer->skybox, camera, render_pass, cmdbuf);
	}
	//render models
	if(renderer->models != NULL)
	{
		for(Uint8 i = 0; i < renderer->num_models; i++)
		{
			Matrix4x4 viewproj;
			viewproj = Matrix4x4_Mul(camera->view, camera->projection);
			Matrix4x4 mvp = Matrix4x4_Mul(renderer->models[i].transform, viewproj);
			drawmodelsimple(&renderer->models[i], mvp, renderer->sampler, render_pass, cmdbuf);
		}
	}

	SDL_EndGPURenderPass(render_pass);
	SDL_SubmitGPUCommandBuffer(cmdbuf);
}