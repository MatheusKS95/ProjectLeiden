#include <demos.h>
#include <SDL3/SDL.h>
#include <graphics.h>

void Demo_ClearScreen()
{
	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	if (cmdbuf == NULL)
	{
		SDL_Log("GpuAcquireCommandBuffer failed");
		return;
	}

	SDL_GPUTexture* swapchainTexture;
	if (!SDL_AcquireGPUSwapchainTexture(cmdbuf, context.window, &swapchainTexture, NULL, NULL))
	{
		SDL_Log("AcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		return;
	}

	if (swapchainTexture != NULL)
	{
		SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
		colorTargetInfo.texture = swapchainTexture;
		colorTargetInfo.clear_color = (SDL_FColor){ 1.0f, 0.4f, 0.6f, 1.0f };
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}