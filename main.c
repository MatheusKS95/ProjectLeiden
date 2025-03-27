/**
 * Project Rockport
 * Copyright 2023 Matheus Klein Schaefer
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <stdio.h>
#include <stdbool.h>
#include <getopt.h> //WARNING: this will make my code non-portable, won't work on windows (except through mingw)
#include <SDL3/SDL.h>
#include <graphics.h>
//#include <audio.h>
#include <fileio.h>
#include <input.h>
#include <ini.h>
#include <demos.h>

int main(int argc, char *argv[])
{
	//COMMAND LINE PARSING
	//---BEGIN------------
	//char *scripts_path = NULL; //will be used by a future lua interpreter
	char *assets_path = NULL; //will be used by physfs
	//char *storefront_plugin = NULL; //will be used to load storefront APIs as plugins
	// Configuração das opções para getopt
	static struct option long_options[] = {
		//{"scripts", required_argument, 0, 's'},
		{"assets", required_argument, 0, 'a'},
		//{"sf", required_argument, 0, 'f'},
		{0, 0, 0, 0}
	};
	int option_index = 0;
	int c;

	while ((c = getopt_long(argc, argv, "s:a:f:", long_options, &option_index)) != -1) {
		switch (c)
		{
			/*case 's':
				scripts_path = optarg;  //Script folder path
				break;*/
			case 'a':
				assets_path = optarg;   //Asset folder/file path
				break;
			/*case 'f':
				storefront_plugin = optarg;  //Storefront API wrapper path (Steam/Epic/GOG/etc.)
				break;*/
			case '?':
				// getopt_long should show up a message if incorrect parameters are passed along
				return -1;
			default:
				return -1;
		}
	}
	//checking if all needed parameters are correct
	//if (!scripts_path || !assets_path || !storefront_plugin)
	if(assets_path == NULL)
	{
		fputs("Use: ProjectLeiden --scripts <path> --assets <path> --sf <plugin>\n", stderr);
		return -1;
	}
	//COMMAND LINE PARSING
	//---END--------------

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

	const int linked = SDL_GetVersion();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Using SDL version %d.%d.%d", SDL_VERSIONNUM_MAJOR(linked), SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked));

	if(!FileIOInit(argv, assets_path, NULL, "Schaefer", "ProjectLeiden"))
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

	//TODO Create window
	bool fullscreen = (INIGetFloat(ini, "graphics", "fullscreen") == 0.0f) ? false : true;
	int width = (int)INIGetFloat(ini, "graphics", "screen_width");
	int height = (int)INIGetFloat(ini, "graphics", "screen_heigth");
	INIDestroy(&ini);
	SDL_Window *window;
	if(fullscreen)
	{
		#ifdef USING_SDLGPU
		window = SDL_CreateWindow("Project Leiden", width, height, SDL_WINDOW_FULLSCREEN);
		#else //USING_OPENGL
		window = SDL_CreateWindow("Project Leiden", width, height, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
		#endif
	}
	else
	{
		#ifdef USING_SDLGPU
		window = SDL_CreateWindow("Project Leiden", width, height, 0);
		#else //USING_SOKOL
		window = SDL_CreateWindow("Project Leiden", width, height, SDL_WINDOW_OPENGL);
		#endif
	}

	if (window == NULL)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "CreateWindow failed: %s", SDL_GetError());
		return -1;
	}
	Graphics_SetContext(window, width, height);
	if(!Graphics_Init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to init graphics module");
		SDL_DestroyWindow(window);
		FileIODeinit();
		SDL_Quit();
		return 0;
	}

	/*if(!Audio_Init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to init audio module");
	}*/

	//test
	/*Audio_LogNameCurrentDevice();
	Music testmusic = { 0 };
	bool testmusicbool = Audio_LoadMusicFromFS(&testmusic, "test_music/change_zephy.flac", true);
	SDL_LogInfo(SDL_LOG_CATEGORY_TEST, "Music loading: this might have worked? %s", testmusicbool ? "yes" : "no");
	Audio_PlayMusic(&testmusic);
	Sound testsound = { 0 };
	bool testsoundbool = Audio_LoadSoundFromFS(&testsound, "test_sound/wind1.wav");
	SDL_LogInfo(SDL_LOG_CATEGORY_TEST, "Sound loading: this might have worked? %s", testsoundbool ? "yes" : "no");
	Audio_PlaySound(&testsound);*/

	//Demo_Set1_Setup();
	//Demo_Set2_Setup();

	InputState state = { 0 };

	SDL_Event event;
	bool playing = true;

	while(playing)
	{
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_EVENT_QUIT)
			{
				SDL_LogInfo(SDL_LOG_CATEGORY_TEST, "Exiting...");
				playing = false;
				break;
			}
			Input_GetState(&state, &event);
		}
		//Demo_Set1_Logic(&state);
		//Demo_Set2_Logic(&state);

		Demo_ClearScreen();
		//Demo_Set1_Draw();
		//Demo_Set2_Draw();

		/*if(status.exit_request)
		{
			SDL_LogInfo(SDL_LOG_CATEGORY_TEST, "Exiting...");
			playing = false;
			break;
		}*/
	}
	/*Audio_StopMusic(&testmusic);
	Audio_FreeMusic(&testmusic);
	Audio_FreeSound(&testsound);
	Audio_Deinit();*/
	Graphics_Deinit();
	SDL_DestroyWindow(window);
	FileIODeinit();
	SDL_Quit();
	return 0;
}
