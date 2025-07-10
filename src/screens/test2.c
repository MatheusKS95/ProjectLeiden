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
#include <screens.h>

bool TestScreen2_Setup()
{
	return true;
}

void TestScreen2_Logic(SDL_Event event)
{
	while(SDL_PollEvent(&event))
	{
		if(event.type == SDL_EVENT_QUIT)
		{
			exit_signal = true;
		}
	}
	return;
}

void TestScreen2_Draw()
{
	SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(drawing_context.device);
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Splash screen: failed to acquire command buffer: %s", SDL_GetError());
		return;
	}
	SDL_GPUTexture *swapchain_texture;
	if(!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, drawing_context.window, &swapchain_texture, NULL, NULL))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: Failed to acquire swapchain texture: %s", SDL_GetError());
		return;
	}
	if(swapchain_texture != NULL)
	{
		SDL_GPUColorTargetInfo colortargetinfo = { 0 };
		colortargetinfo.texture = swapchain_texture;
		colortargetinfo.clear_color = (SDL_FColor){ 0.0f, 1.0f, 0.0f, 1.0f };
		colortargetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colortargetinfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colortargetinfo, 1, NULL);
		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
	return;
}

void TestScreen2_Destroy()
{
	return;
}
