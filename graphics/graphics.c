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

#include <graphics.h>
#include <fileio.h>

GraphicsContext context;
GeneralPipelines pipelines;
DefaultTextures default_textures;

bool Graphics_Init()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Graphics: Initializing graphics module");

	context.device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, false);

	if (context.device == NULL)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Failed to init GPU device.");
		return false;
	}

	if (!SDL_ClaimWindowForGPUDevice(context.device, context.window))
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Failed to claim window");
		return false;
	}

	pipelines = (GeneralPipelines){ 0 };

	return true;
}

void Graphics_Deinit()
{
	SDL_ReleaseWindowFromGPUDevice(context.device, context.window);
	SDL_DestroyGPUDevice(context.device);
	return;
}

void Graphics_SetContext(SDL_Window *window,
							unsigned int width,
							unsigned int height)
{
	if(window == NULL || width <= 0 || height <= 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Graphics_SetContext set with invalid parameters");
		return;
	}
	context.device = NULL;
	context.window = window;
	context.width = width;
	context.height = height;
}
