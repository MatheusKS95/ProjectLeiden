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
#include <joltc.h>
#include <fileio.h>
#include <ini.h>
#include <screens.h>

int main(int argc, char *argv[])
{
	SDL_Log("Copyright (C) 2025 Matheus Klein Schaefer - All rights reserved.");
	SDL_Log("TODO GPL notice");
	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ERROR: %s", SDL_GetError());
		return -1;
	}
	if(!SDL_InitSubSystem(SDL_INIT_EVENTS))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ERROR: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Current platform: %s", SDL_GetPlatform());
	const int linked = SDL_GetVersion();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
					"Using SDL version %d.%d.%d",
					SDL_VERSIONNUM_MAJOR(linked),
					SDL_VERSIONNUM_MINOR(linked),
					SDL_VERSIONNUM_MICRO(linked));

	if(!FileIOInit(argv, argv[1], NULL, "Schaefer", "ProjectLeiden"))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Content not found. Stopping.");
		SDL_Quit();
		return -1;
	}

	//TODO improve this, this is terrible (but works)
	int conf_found = 0;
	INIstruct *ini = ININew();
	if(!INILoad(ini, "settings/settings.ini"))
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "Failed to read default default config files.");
		conf_found++;
	}
	if(!INILoad(ini, "usersettings.ini"))
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "Failed to read user config files.");
		conf_found++;
	}
	if(conf_found >= 2)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to set configurations. Unable to continue.");
		FileIODeinit();
		SDL_Quit();
		return -1;
	}

	INISave(ini, "usersettings.ini");

	bool fullscreen = (INIGetFloat(ini, "graphics", "fullscreen") == 0.0f) ? false : true;
	int width = (int)INIGetFloat(ini, "graphics", "screen_width");
	int height = (int)INIGetFloat(ini, "graphics", "screen_heigth");

	SDL_Window *window;
	if(fullscreen)
	{
		window = SDL_CreateWindow("Project Leiden", width, height, SDL_WINDOW_FULLSCREEN);
	}
	else
	{
		window = SDL_CreateWindow("Project Leiden", width, height, 0);
	}

	if (window == NULL)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "CreateWindow failed: %s", SDL_GetError());
		return -1;
	}

	//when release, should be false (the first boolean)
	SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, false);

	if (device == NULL)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Failed to init GPU device.");
		return -1;
	}

	if (!SDL_ClaimWindowForGPUDevice(device, window))
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Failed to claim window");
		return -1;
	}

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "GPU API: %s", SDL_GetGPUDeviceDriver(device));

	INIDestroy(&ini);

	//init physics
	if(!JPH_Init())
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Failed to init joltc");
		return -1;
	}

	//more stuff
	SCR_SetContext(window, device);
	SCR_Setup();

	SDL_Event event;
	bool playing = true;

	while(playing)
	{
		SCR_Logic(event);

		SCR_Draw();

		if(exit_signal)
		{
			playing = false;
			break;
		}
	}

	SCR_Destroy();
	FileIODeinit();
	JPH_Shutdown();
	SDL_ReleaseWindowFromGPUDevice(device, window);
	SDL_DestroyGPUDevice(device);
	SDL_Quit();
	return 0;
}
