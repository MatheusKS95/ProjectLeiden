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
	CommandBuffer *cmdbuf = Graphics_SetupCommandBuffer();
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Splash screen: failed to acquire command buffer: %s", SDL_GetError());
		return;
	}

	GPUTexture *swapchain_texture = Graphics_AcquireSwapchainTexture(cmdbuf);

	if (swapchain_texture != NULL)
	{
		Color clearcolor = { 1.0f, 0.5f, 0.0f, 1.0f };
		RenderPass *renderpass = Graphics_BeginRenderPass(cmdbuf, swapchain_texture, NULL, clearcolor);
		Graphics_EndRenderPass(renderpass);
	}

	Graphics_CommitCommandBuffer(cmdbuf);
	return;
}

void SplashScreen_Destroy()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing splash screen...");
	return;
}
