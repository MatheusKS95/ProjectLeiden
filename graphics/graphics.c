#include <graphics.h>
#include <fileio.h>

GraphicsContext context;

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

	return true;
}

void Graphics_Deinit()
{
	//shutdown
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
