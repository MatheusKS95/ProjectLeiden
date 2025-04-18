/**
 * Project Rockport
 * Copyright 2023 Matheus Klein Schaefer
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <stdio.h>
#include <getopt.h> //WARNING: this will make my code non-portable, won't work on windows (except through mingw)
#include <SDL3/SDL.h>
#include <leiden.h>
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

	LeidenInitDesc desc = { 0 };
	SDL_snprintf(desc.app_name, 64, "ProjectLeiden");
	SDL_snprintf(desc.org_name, 64, "Schaefer");
	desc.argc = argc;
	desc.argv = argv;
	SDL_snprintf(desc.asset_path, 128, "%s", assets_path);
	Leiden_Init(&desc);

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
	}

	//Demo_Set1_Destroy();
	//Demo_Set2_Destroy();

	Leiden_Deinit();

	return 0;
}
