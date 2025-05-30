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

#include <stdio.h>
#include <getopt.h> //WARNING: this will make my code non-portable, won't work on windows (except through mingw - checked)
#include <SDL3/SDL.h>
#include <leiden.h>
#include <screens.h>

int main(int argc, char *argv[])
{
	//COMMAND LINE PARSING
	//---BEGIN------------
	char *assets_path = NULL; //will be used by physfs
	//char *storefront_plugin = NULL; //if i ever use dll/so plugins for storefront shit, otherwise steamshim and I'm done
	//getopt options config
	static struct option long_options[] = {
		{"assets", required_argument, 0, 'a'},
		//{"sf", required_argument, 0, 'f'},
		{0, 0, 0, 0}
	};
	int option_index = 0;
	int c;

	while ((c = getopt_long(argc, argv, "a:f:", long_options, &option_index)) != -1) {
		switch (c)
		{
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
	//if (!assets_path || !storefront_plugin)
	if(assets_path == NULL)
	{
		fputs("Use: ProjectLeiden --assets <path> --sf <plugin>\n", stderr);
		return -1;
	}
	//COMMAND LINE PARSING
	//---END--------------

	LeidenInitDesc desc = { 0 };
	SDL_snprintf(desc.app_name, 64, "ProjectLeiden");
	SDL_snprintf(desc.org_name, 64, "Schaefer");
	desc.argc = argc;
	desc.argv = argv;
	SDL_snprintf(desc.asset_path, 128, "%s", assets_path);
	Leiden_Init(&desc);

	//Simple_Setup();
	//SplashScreen_Setup();
	SCR_Setup();

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

		/************************
		 * LOGIC STUFF **********
		 ***********************/
		//Simple_Logic(&state);
		//SplashScreen_Logic(&state);
		SCR_Logic(&state);

		/************************
		 * RENDERING STUFF ******
		 ***********************/
		//Simple_Draw();
		//SplashScreen_Draw();
		SCR_Draw();

		/************************************/
	}

	//i forgor more things to kill
	//valgrind is going to scream
	//Simple_Destroy();
	//SplashScreen_Destroy();
	Leiden_Deinit();

	return 0;
}
