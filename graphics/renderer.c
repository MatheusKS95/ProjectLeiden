/**
 * @file renderer.c
 * @brief Central implementation for rendering stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <graphics.h>
#include <SDL3/SDL.h>

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

void Graphics_DrawModel(Model *model, Renderer *renderer,
						Matrix4x4 mvp, Sampler *sampler)
{
	if(model == NULL || renderer == NULL)
	{
		return;
	}

	for(int i = 0; i < model->meshes.count; ++i)
	{
		SDL_BindGPUGraphicsPipeline(renderer->render_pass, model->pipeline);
		SDL_BindGPUVertexBuffers(renderer->render_pass, 0, &(SDL_GPUBufferBinding){ model->meshes.meshes[i].vbuffer, 0 }, 1);
		SDL_BindGPUIndexBuffer(renderer->render_pass, &(SDL_GPUBufferBinding){ model->meshes.meshes[i].ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);
		SDL_BindGPUFragmentSamplers(renderer->render_pass, 0, &(SDL_GPUTextureSamplerBinding){ model->meshes.meshes[i].material.textures[TEXTURE_DIFFUSE]->texture, sampler }, 1);
		SDL_PushGPUVertexUniformData(renderer->cmdbuf, 0, &mvp, sizeof(mvp));
		SDL_DrawGPUIndexedPrimitives(renderer->render_pass, model->meshes.meshes[i].indices.count, 1, 0, 0, 0);
	}
}