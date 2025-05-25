//i am going raylib route, plus some macgyver crap i will regret later
//https://www.youtube.com/watch?v=yOEe1uzurKo

/*
 * CURRENTLY RUNS ON
 * > GNU/Linux: developed on it, duh. Vulkan only.
 * > GhostBSD: no change needed, as long it's not Nvidia (driver issues). Vulkan only.
 * > Windows (MinGW-W64): no change needed. Guess what? Vulkan only.
 *
 * NEED TO CHECK
 * > FreeBSD: GhostBSD is based on it, so probably runs without modifications. It'll be Vulkan only.
 * > Windows (MSVC): didn't work with CMake directly, gonna try generate builds separately first,
 *   but getopt will get in the way. If I manage to get it working, let's see if it works as DX12.
 *
 * WON'T RUN, NO MATTER HOW
 * > MacOS: no access to Mac, Apple don't support SPIR-V. Probably require a lot of work to make
 *   it run. SDL_shadercross could help, but it's temperamental and might give issues.
 * > iOS: same as MacOS. Probably even more headache.
 * > Android: people reported headache when working with SDL GPU on Android. PhysFS might also be
 *   a problem.
 * > Web: SDL GPU doesn't have a web backend yet (WebGPU is WIP and will be for a long time).
 *   PhysFS also might be an issue.
*/

//this file is temporary until the API is close to completion

#include <stdio.h>
#include <getopt.h> //WARNING: this will make my code non-portable, won't work on windows (except through mingw - checked)
#include <SDL3/SDL.h>
#include <leiden.h>

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

	float deltatime;
	float lastframe;
	float last_x, last_y;
	float mouse_x, mouse_y;
	bool first_mouse;
	Camera cam_1;

	deltatime = lastframe = 0.0f;
	last_x = context.width / 2.0f;
	last_y = context.height / 2.0f;
	mouse_x = last_x;
	mouse_y = last_y;
	first_mouse = true;
	//need to bring lookat, or a camera update thing
	Graphics_InitCameraBasic(&cam_1, (Vector3){0.0f, 0.5f, 0.0f});

	//skybox test
	Skybox skybox = { 0 };
	if(!Graphics_LoadSkyboxFS(&skybox, "skybox/exosystem/top.jpg", "skybox/exosystem/bottom.jpg",
								"skybox/exosystem/left.jpg", "skybox/exosystem/right.jpg",
								"skybox/exosystem/front.jpg", "skybox/exosystem/back.jpg"))
	{
		//todo cleanup this
		return -1;
	}
	Graphics_UploadSkybox(&skybox);

	Model *house = (Model*)SDL_malloc(sizeof(Model));
	if(house != NULL)
	{
		Graphics_ImportIQM(house, "test_models/house/house.iqm");
		Graphics_LoadModelMaterials(house, "test_models/house/house.material");
		Graphics_UploadModel(house, true);
		Graphics_MoveModel(house, (Vector3){10.0f, 0.0f, 5.0f});
	}

	Model *vroid_test = (Model*)SDL_malloc(sizeof(Model));
	if(vroid_test != NULL)
	{
		Graphics_ImportIQM(vroid_test, "test_models/avatarsamplek_teste/avatarsamplek.iqm");
		Graphics_LoadModelMaterials(vroid_test, "test_models/avatarsamplek_teste/avatarsamplek.material");
		Graphics_UploadModel(vroid_test, true);
		Graphics_RotateModel(vroid_test, (Vector3){0.0f, 1.0f, 0.0f}, DegToRad(120));
		Graphics_MoveModel(vroid_test, (Vector3){0.0f, 0.0f, 2.0f});
	}

	//TODO: the correct order for transform a model is scale > rotation > translation
	//this is the issue i have when rotating it using the deltatime and begin orbiting the middle at mach speeds
	//keep this in mind when doing stuff in the loop - besides, transform shouldn't be here at all, but...

	Sampler *sampler = Graphics_GenerateSampler(SAMPLER_FILTER_LINEAR, SAMPLER_MODE_CLAMPTOEDGE);

	InputState state = { 0 };

	Graphics_PrepareSimpleRendering();

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
		//TODO should be moved somewhere, stuff don't have a home
		float current_frame = (float)SDL_GetTicks();
		deltatime = current_frame - lastframe;
		lastframe = current_frame;
		float velocity = 0.01f * deltatime;
		if(Input_IsKeyPressedTEMP(&state, SDL_SCANCODE_RIGHT))
		{
			Vector3 aux = { 0 };
			aux.x = cam_1.right.x * velocity;
			aux.y = cam_1.right.y * velocity;
			aux.z = cam_1.right.z * velocity;
			Graphics_UpdateCameraPosition(&cam_1, Vector3_Add(cam_1.position, aux));
		}
		if(Input_IsKeyPressedTEMP(&state, SDL_SCANCODE_LEFT))
		{
			Vector3 aux = { 0 };
			aux.x = cam_1.right.x * velocity;
			aux.y = cam_1.right.y * velocity;
			aux.z = cam_1.right.z * velocity;
			Vector3 newpos = cam_1.position;
			newpos.x = newpos.x - aux.x;
			newpos.y = newpos.y - aux.y;
			newpos.z = newpos.z - aux.z;
			Graphics_UpdateCameraPosition(&cam_1, newpos);
		}
		if(first_mouse)
		{
			last_x = mouse_x;
			last_y = mouse_y;
			first_mouse = false;
		}
		//i only did keyboard stuff but i forgot mouse lmao
		//i'll be very sad when working with gamepads
		//steering wheels will be truly a painful experience
		mouse_x = state.mouse_x;
		mouse_y = state.mouse_y;
		float x_offset = state.mouse_x + last_x;
		float y_offset = last_y + state.mouse_y;
		last_x = state.mouse_x;
		last_y = state.mouse_y;
		Graphics_TestCameraFreecam(&cam_1, x_offset, y_offset, true);
		state.mouse_x = state.mouse_y = 0;

		/************************
		 * RENDERING STUFF ******
		 ***********************/
		//just a test, need to make sure if models are loaded correctly and are not null
		Model models[2];
		models[0] = *house;
		models[1] = *vroid_test;
		SimpleRenderingSetup testsimple = { 0 };
		testsimple.models = models;
		testsimple.num_models = 2;
		testsimple.sampler = sampler;
		testsimple.skybox = &skybox;

		Graphics_DrawSimple(&testsimple, (Color){0.0f, 0.0f, 0.0f, 0.0f}, &cam_1);
		/************************************/
	}

	//i forgor more things to kill
	//valgrind is going to scream
	Graphics_ReleaseModel(house);
	Graphics_ReleaseModel(vroid_test);
	SDL_free(house);
	SDL_free(vroid_test);
	Graphics_ReleaseSampler(sampler);
	Graphics_FinishSimpleRendering();
	Leiden_Deinit();

	return 0;
}
