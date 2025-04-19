//i am going raylib route, plus some macgyver crap i will regret later
//https://www.youtube.com/watch?v=yOEe1uzurKo

#include <stdio.h>
#include <getopt.h> //WARNING: this will make my code non-portable, won't work on windows (except through mingw)
#include <SDL3/SDL.h>
#include <leiden.h>
#include <demos.h>

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
	Model model1 = { 0 };
	Model model2 = { 0 };
	Sampler *sampler;

	deltatime = lastframe = 0.0f;
	last_x = context.width / 2.0f;
	last_y = context.height / 2.0f;
	mouse_x = last_x;
	mouse_y = last_y;
	first_mouse = true;
	//need to bring lookat, or a camera update thing
	Graphics_InitCameraBasic(&cam_1, (Vector3){1.0f, 1.0f, 8.0f});
	Shader modelvsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&modelvsshader, "shaders/demoset2.vert.spv", "main", SHADERSTAGE_VERTEX, 0, 1, 0, 0))
	{
		SDL_Log("Failed to load VS shader.");
		return -1;
	}
	Shader modelfsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&modelfsshader, "shaders/demoset2.frag.spv", "main", SHADERSTAGE_FRAGMENT, 1, 0, 0, 0))
	{
		SDL_Log("Failed to load FS shader.");
		return -1;
	}
	//TODO model don't release it, but i forgot to make something to destroy this
	Pipeline pipeline1 = Graphics_CreatePipeline(&modelvsshader, &modelfsshader, PIPELINETYPE_3D, true);

	sampler = Graphics_GenerateSampler(SAMPLER_FILTER_LINEAR, SAMPLER_MODE_CLAMPTOEDGE);

	if(!Graphics_ImportIQM(&model1, "test_models/house/house.iqm", "test_models/house/house.material", pipeline1))
	{
		//todo cleanup this shit
		return -1;
	}

	if(!Graphics_ImportIQM(&model2, "test_models/avatarsampleb_teste/avatarsampleb.iqm", "test_models/avatarsampleb_teste/avatarsampleb.material", pipeline1))
	{
		//todo cleanup this shit
		return -1;
	}

	Graphics_UploadModel(&model1, true);
	Graphics_UploadModel(&model2, true);

	Graphics_MoveModel(&model1, (Vector3){0.0f, 0.0f, 0.0f});

	Graphics_RotateModel(&model2, (Vector3){1.0f, 0.0f, 0.0f}, DegToRad(-90));
	Graphics_MoveModel(&model2, (Vector3){1.0f, 0.0f, 4.0f});
	//Graphics_ScaleModel(&model2, 2.0f);

	InputState state = { 0 };

	SDL_Event event;
	bool playing = true;

	Renderer renderer = { 0 };
	Graphics_CreateRenderer(&renderer, (Color){0.0f, 0.0f, 0.0f, 0.0f});

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

		//https://www.youtube.com/watch?v=PGNiXGX2nLU
		Graphics_RotateModel(&model1, (Vector3){0.0f, 1.0f, 0.0f}, DegToRad(deltatime / 10));

		Matrix4x4 viewproj;
		viewproj = Matrix4x4_Mul(cam_1.view, cam_1.projection);
		Matrix4x4 mvp1 = Matrix4x4_Mul(model1.transform, viewproj);
		Matrix4x4 mvp2 = Matrix4x4_Mul(model2.transform, viewproj);
		/************************************/

		/************************
		 * RENDERING STUFF ******
		 ***********************/
		Graphics_BeginDrawing(&renderer);
		Graphics_DrawModel(&model1, &renderer, mvp1, sampler);
		Graphics_DrawModel(&model2, &renderer, mvp2, sampler);
		Graphics_EndDrawing(&renderer);
		//TODO: see raylib cheatsheet, that's the todo list
		/************************************/
	}

	//i forgor more things to kill
	//valgrind is going to peg me
	Graphics_ReleaseModel(&model1); //at least this destroy textures
	Graphics_ReleaseSampler(sampler);
	//TODO release pipeline

	Leiden_Deinit();

	return 0;
}
