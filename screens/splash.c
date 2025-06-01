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
#include <leiden.h>

//TODO render texture, later a short video or something
//also preload the simple test screen

bool SplashScreen_Setup()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting splash screen...");
	return true;
}

void SplashScreen_Logic(InputState *state)
{
	if(Input_IsKeyPressedTEMP(state, SDL_SCANCODE_RETURN))
	{
		if(Simple_Setup())
		{
			SplashScreen_Destroy();
			current_screen = SCREEN_DEMOSIMPLE;
		}
	}
	return;
}

void SplashScreen_Draw()
{
	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Splash screen: failed to acquire command buffer: %s", SDL_GetError());
		return;
	}

	SDL_GPUTexture* swapchain_texture;
	if(!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.window, &swapchain_texture, NULL, NULL))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Splash screen: failed to acquire swapchain texture: %s", SDL_GetError());
		return;
	}

	if (swapchain_texture != NULL)
	{
		SDL_GPUColorTargetInfo colortargetinfo = { 0 };
		colortargetinfo.texture = swapchain_texture;
		colortargetinfo.clear_color = (SDL_FColor){ 1.0f, 0.5f, 0.0f, 1.0f };
		colortargetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colortargetinfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(cmdbuf, &colortargetinfo, 1, NULL);
		SDL_EndGPURenderPass(renderpass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
	return;
}

void SplashScreen_Destroy()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing splash screen...");
	return;
}
