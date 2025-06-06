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

#include <screens.h>
#include <leiden.h>

bool DemoPostProc_Setup()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting postproc screen...");
	return true;
}

void DemoPostProc_Logic(InputState *state)
{
	if(Input_IsKeyPressedTEMP(state, SDL_SCANCODE_ESCAPE))
	{
		//TEST, remove this later
		current_screen = SCREEN_SPLASH;
		DemoPostProc_Destroy();
	}
	return;
}

void DemoPostProc_Draw()
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
		Color clearcolor = { 0.0f, 0.0f, 0.8f, 1.0f };
		RenderPass *renderpass = Graphics_BeginRenderPass(cmdbuf, swapchain_texture, NULL, clearcolor);
		Graphics_EndRenderPass(renderpass);
	}

	Graphics_CommitCommandBuffer(cmdbuf);
	return;
}

void DemoPostProc_Destroy()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing postproc screen...");
	return;
}