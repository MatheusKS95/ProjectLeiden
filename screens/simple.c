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

struct SimpleRenderingSetup
{
	Model *models;
	Uint8 num_models;
	Skybox *skybox;
	Sampler *sampler;
};

//should be moved to leiden.h
static float deltatime;
static float lastframe;

static float last_x, last_y;
static float mouse_x, mouse_y;
static bool first_mouse;
static Camera cam_1;

static Skybox *skybox;
static Model *house;
static Model *vroid_test;
static Model *mulher;
static Sampler *sampler;
static GPUTexture *depth_texture;
static Pipeline *simple_pipeline;

static void drawskybox(Skybox *skybox, Camera *camera, RenderPass *render_pass, CommandBuffer *cmdbuf)
{
	if(skybox == NULL || camera == NULL || render_pass == NULL || cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't render skybox.");
		return;
	}
	Matrix4x4 cam_view = camera->view;
	cam_view.ad = cam_view.bd = cam_view.cd = cam_view.dd = 0.0f;
	cam_view.da = cam_view.db = cam_view.dc = 0.0f;
	Matrix4x4 skyboxviewproj;
	skyboxviewproj = Matrix4x4_Mul(cam_view, camera->projection);

	Graphics_BindPipeline(render_pass, skybox->pipeline);
	Graphics_BindVertexBuffers(render_pass, skybox->vertex_buffer, 0, 0, 1);
	Graphics_BindIndexBuffers(render_pass, skybox->index_buffer, 0);
	Graphics_BindFragmentSampledGPUTexture(render_pass, skybox->gputexture, skybox->sampler, 0, 1);
	Graphics_PushVertexUniforms(cmdbuf, 0, &skyboxviewproj, sizeof(skyboxviewproj));
	Graphics_DrawPrimitives(render_pass, 36, 1, 0, 0, 0);
}

static void drawmodelsimple(Model *model, Matrix4x4 mvp, Sampler *sampler, RenderPass *render_pass, CommandBuffer *cmdbuf)
{
	if(model == NULL || render_pass == NULL || cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't render model.");
		return;
	}

	for(size_t i = 0; i < model->meshes.count; i++)
	{
		Mesh *mesh = &model->meshes.meshes[i];
		//binding graphics pipeline
		Graphics_BindPipeline(render_pass, simple_pipeline);

		//binding vertex and index buffers
		Graphics_BindMeshBuffers(render_pass, mesh);

		//texture samplers
		Material *material = Graphics_GetMaterialByName(&model->materials, mesh->material_name);
		if(material == NULL) continue;
		Texture2D *diffuse = material->diffuse_map;
		Graphics_BindFragmentSampledTexture(render_pass, diffuse, sampler, 0, 1);

		//UBO
		Graphics_PushVertexUniforms(cmdbuf, 0, &mvp, sizeof(mvp));

		Graphics_DrawPrimitives(render_pass, mesh->indices.count, 1, 0, 0, 0);
	}
}

static void simpledraw(struct SimpleRenderingSetup *stuff,
						Color clear_color,
						Camera *camera)
{
	if(stuff == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't render anything. Make sure you send the correct structure.");
		return;
	}

	CommandBuffer *cmdbuf = Graphics_SetupCommandBuffer();
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to acquire command buffer.");
		return;
	}

	GPUTexture *swapchain_texture = Graphics_AcquireSwapchainTexture(cmdbuf);

	if(swapchain_texture == NULL)
	{
		return;
	}

	RenderPass *render_pass = Graphics_BeginRenderPass(cmdbuf, swapchain_texture, depth_texture, clear_color);

	//render skybox
	if(stuff->skybox != NULL)
	{
		drawskybox(stuff->skybox, camera, render_pass, cmdbuf);
	}
	//render models
	if(stuff->models != NULL)
	{
		for(Uint8 i = 0; i < stuff->num_models; i++)
		{
			Matrix4x4 viewproj;
			viewproj = Matrix4x4_Mul(camera->view, camera->projection);
			Matrix4x4 mvp = Matrix4x4_Mul(stuff->models[i].transform, viewproj);
			drawmodelsimple(&stuff->models[i], mvp, stuff->sampler, render_pass, cmdbuf);
		}
	}

	Graphics_EndRenderPass(render_pass);
	Graphics_CommitCommandBuffer(cmdbuf);
}

/***************************************************************************************
 * end of static stuff
 **************************************************************************************/

bool Simple_Setup()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting simple test screen...");
	deltatime = lastframe = 0.0f;
	last_x = context.width / 2.0f;
	last_y = context.height / 2.0f;
	mouse_x = last_x;
	mouse_y = last_y;
	first_mouse = true;
	//need to bring lookat, or a camera update thing
	Graphics_InitCameraBasic(&cam_1, (Vector3){0.0f, 0.5f, 0.0f});

	skybox = (Skybox*)SDL_malloc(sizeof(Skybox));
	if(skybox != NULL)
	{
		if(!Graphics_LoadSkyboxFS(skybox, "skybox/exosystem/top.jpg", "skybox/exosystem/bottom.jpg",
									"skybox/exosystem/left.jpg", "skybox/exosystem/right.jpg",
									"skybox/exosystem/front.jpg", "skybox/exosystem/back.jpg"))
		{
			//todo cleanup this
			return false;
		}
		Graphics_UploadSkybox(skybox);
		//FIXME this returns a boolean, should check first if worked or not...
		Graphics_CreatePipelineSkybox(skybox, "shaders/skybox/skybox.vert.spv", "shaders/skybox/skybox.frag.spv");
	}

	Shader *vsshader = Graphics_LoadShader("shaders/simple/simple.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	if(vsshader == NULL)
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return NULL;
	}
	Shader *fsshader = Graphics_LoadShader("shaders/simple/simple.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(fsshader == NULL)
	{
		SDL_Log("Failed to load skybox fragment shader.");
		return NULL;
	}
	simple_pipeline = Graphics_Generate3DPipelineFull(vsshader, fsshader, true);

	house = (Model*)SDL_malloc(sizeof(Model));
	if(house != NULL)
	{
		Graphics_ImportIQM(house, "test_models/house/house.iqm");
		Graphics_LoadModelMaterials(house, "test_models/house/house.material");
		Graphics_UploadModel(house, true);
		Graphics_MoveModel(house, (Vector3){10.0f, 0.0f, 5.0f});
	}

	vroid_test = (Model*)SDL_malloc(sizeof(Model));
	if(vroid_test != NULL)
	{
		Graphics_ImportIQM(vroid_test, "test_models/michikocasual/michikocasual.iqm");
		Graphics_LoadModelMaterials(vroid_test, "test_models/michikocasual/michikocasual.material");
		Graphics_UploadModel(vroid_test, true);
		Graphics_RotateModel(vroid_test, (Vector3){1.0f, 0.0f, 0.0f}, DegToRad(90)); //iqm blender exporter problems
		Graphics_RotateModel(vroid_test, (Vector3){0.0f, 1.0f, 0.0f}, DegToRad(170));
		Graphics_MoveModel(vroid_test, (Vector3){0.0f, 0.0f, 2.0f});
	}

	mulher = (Model*)SDL_malloc(sizeof(Model));
	if(mulher != NULL)
	{
		Graphics_ImportIQM(mulher, "test_models/mulher2/mulher_face2.iqm");
		Graphics_LoadModelMaterials(mulher, "test_models/mulher2/mulher_face2.material");
		Graphics_UploadModel(mulher, true);
		Graphics_ScaleModel(mulher, 0.1);
		Graphics_MoveModel(mulher, (Vector3){3.0f, 0.0f, 2.0f});
	}

	//TODO: the correct order for transform a model is scale > rotation > translation
	//this is the issue i have when rotating it using the deltatime and begin orbiting the middle at mach speeds
	//keep this in mind when doing stuff in the loop - besides, transform shouldn't be here at all, but...

	sampler = Graphics_GenerateSampler(SAMPLER_FILTER_LINEAR, SAMPLER_MIPMAPMODE_LINEAR, SAMPLER_MODE_CLAMPTOEDGE);

	depth_texture = Graphics_GenerateDepthTexture(context.width, context.height);

	return true;
}

void Simple_Logic(InputState *state)
{
	float current_frame = (float)SDL_GetTicks();
	deltatime = current_frame - lastframe;
	lastframe = current_frame;
	float velocity = 0.01f * deltatime;
	if(Input_IsKeyPressedTEMP(state, SDL_SCANCODE_RIGHT))
	{
		Vector3 aux = { 0 };
		aux.x = cam_1.right.x * velocity;
		aux.y = cam_1.right.y * velocity;
		aux.z = cam_1.right.z * velocity;
		Graphics_UpdateCameraPosition(&cam_1, Vector3_Add(cam_1.position, aux));
	}
	if(Input_IsKeyPressedTEMP(state, SDL_SCANCODE_LEFT))
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
	if(Input_IsKeyPressedTEMP(state, SDL_SCANCODE_ESCAPE))
	{
		//TEST, remove this later
		current_screen = SCREEN_SPLASH;
		Simple_Destroy();
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
	mouse_x = state->mouse_x;
	mouse_y = state->mouse_y;
	float x_offset = state->mouse_x + last_x;
	float y_offset = last_y + state->mouse_y;
	last_x = state->mouse_x;
	last_y = state->mouse_y;
	Graphics_TestCameraFreecam(&cam_1, x_offset, y_offset, true);
	state->mouse_x = state->mouse_y = 0;
	return;
}

void Simple_Draw()
{
	Model models[3];
	models[0] = *house;
	models[1] = *vroid_test;
	models[2] = *mulher;
	struct SimpleRenderingSetup testsimple = { 0 };
	testsimple.models = models;
	testsimple.num_models = 3;
	testsimple.sampler = sampler;
	testsimple.skybox = skybox;

	simpledraw(&testsimple, (Color){0.0f, 0.0f, 0.0f, 0.0f}, &cam_1);
	return;
}

void Simple_Destroy()
{
	Graphics_ReleaseModel(house);
	Graphics_ReleaseModel(vroid_test);
	Graphics_ReleaseModel(mulher);
	SDL_free(house);
	SDL_free(vroid_test);
	SDL_free(mulher);
	Graphics_ReleaseSampler(sampler);
	Graphics_ReleaseSkybox(skybox);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing simple test screen...");
	return;
}
