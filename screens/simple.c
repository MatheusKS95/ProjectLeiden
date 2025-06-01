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
static SDL_GPUTexture *depth_texture;
static SDL_GPUGraphicsPipeline *simple_pipeline;

static SDL_GPUGraphicsPipeline *create_pipeline(const char *path_vs, const char *path_fs)
{
	SDL_GPUShader *vsshader = Graphics_LoadShader(path_vs, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	if(vsshader == NULL)
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return NULL;
	}
	SDL_GPUShader *fsshader = Graphics_LoadShader(path_fs, SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(fsshader == NULL)
	{
		SDL_Log("Failed to load skybox fragment shader.");
		return NULL;
	}

	SDL_GPUGraphicsPipelineCreateInfo pipeline_createinfo = { 0 };
	pipeline_createinfo = (SDL_GPUGraphicsPipelineCreateInfo)
	{
		.target_info =
		{
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
				.format = SDL_GetGPUSwapchainTextureFormat(context.device, context.window)
			}},
			.has_depth_stencil_target = true,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM
		},
		.depth_stencil_state = (SDL_GPUDepthStencilState){
			.enable_depth_test = true,
			.enable_depth_write = true,
			.enable_stencil_test = false,
			.compare_op = SDL_GPU_COMPAREOP_LESS,
			.write_mask = 0xFF
		},
		.rasterizer_state = (SDL_GPURasterizerState){
			.cull_mode = SDL_GPU_CULLMODE_NONE,
			.fill_mode = SDL_GPU_FILLMODE_FILL,
			.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
		},
		.vertex_input_state = (SDL_GPUVertexInputState){
			.num_vertex_buffers = 1,
			.vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{
				.slot = 0,
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0,
				.pitch = sizeof(Vertex)
			}},
			.num_vertex_attributes = 2,
			.vertex_attributes = (SDL_GPUVertexAttribute[]){{
				//position
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 0,
				.offset = 0
			}, {
				//uv
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
				.location = 1,
				.offset = (sizeof(float) * 3)
			}}
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vsshader,
		.fragment_shader = fsshader
	};
	SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	SDL_ReleaseGPUShader(context.device, vsshader);
	SDL_ReleaseGPUShader(context.device, fsshader);

	return pipeline;
}

static void drawskybox(Skybox *skybox, Camera *camera, SDL_GPURenderPass *render_pass, SDL_GPUCommandBuffer *cmdbuf)
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

	SDL_BindGPUGraphicsPipeline(render_pass, skybox->pipeline);
	SDL_BindGPUVertexBuffers(render_pass, 0, &(SDL_GPUBufferBinding){ skybox->vertex_buffer, 0 }, 1);
	SDL_BindGPUIndexBuffer(render_pass, &(SDL_GPUBufferBinding){ skybox->index_buffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &(SDL_GPUTextureSamplerBinding){ skybox->gputexture, skybox->sampler }, 1);
	SDL_PushGPUVertexUniformData(cmdbuf, 0, &skyboxviewproj, sizeof(skyboxviewproj));
	SDL_DrawGPUIndexedPrimitives(render_pass, 36, 1, 0, 0, 0);
}

static void drawmodelsimple(Model *model, Matrix4x4 mvp, Sampler *sampler, SDL_GPURenderPass *render_pass, SDL_GPUCommandBuffer *cmdbuf)
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
		SDL_BindGPUGraphicsPipeline(render_pass, simple_pipeline);

		//binding vertex and index buffers
		SDL_BindGPUVertexBuffers(render_pass, 0, &(SDL_GPUBufferBinding){ mesh->vbuffer, 0 }, 1);
		SDL_BindGPUIndexBuffer(render_pass, &(SDL_GPUBufferBinding){ mesh->ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);

		//texture samplers
		Material *material = Graphics_GetMaterialByName(&model->materials, mesh->material_name);
		if(material == NULL) continue;
		Texture2D *diffuse = material->diffuse_map != NULL ? material->diffuse_map : &default_textures.default_diffuse;
		SDL_BindGPUFragmentSamplers(render_pass, 0, &(SDL_GPUTextureSamplerBinding){ diffuse->texture, sampler }, 1);

		//UBO
		SDL_PushGPUVertexUniformData(cmdbuf, 0, &mvp, sizeof(mvp));
		SDL_DrawGPUIndexedPrimitives(render_pass, mesh->indices.count, 1, 0, 0, 0);
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

	SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to acquire command buffer.");
		return;
	}

	SDL_GPUTexture *swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.window, &swapchain_texture, NULL, NULL))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to acquire swapchain texture: %s", SDL_GetError());;
	}

	if(swapchain_texture == NULL)
	{
		return;
	}

	SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
	colorTargetInfo.texture = swapchain_texture;
	colorTargetInfo.clear_color = (SDL_FColor){ clear_color.r, clear_color.g, clear_color.b, clear_color.a };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = { 0 };
	depthStencilTargetInfo.texture = depth_texture;
	depthStencilTargetInfo.cycle = true;
	depthStencilTargetInfo.clear_depth = 1;
	depthStencilTargetInfo.clear_stencil = 0;
	depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
	depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthStencilTargetInfo);

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

	SDL_EndGPURenderPass(render_pass);
	SDL_SubmitGPUCommandBuffer(cmdbuf);
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

	simple_pipeline = create_pipeline("shaders/simple/simple.vert.spv", "shaders/simple/simple.frag.spv");

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
		Graphics_ImportIQM(vroid_test, "test_models/avatarsamplek_teste/avatarsamplek.iqm");
		Graphics_LoadModelMaterials(vroid_test, "test_models/avatarsamplek_teste/avatarsamplek.material");
		Graphics_UploadModel(vroid_test, true);
		Graphics_RotateModel(vroid_test, (Vector3){0.0f, 1.0f, 0.0f}, DegToRad(120));
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

	sampler = Graphics_GenerateSampler(SAMPLER_FILTER_LINEAR, SAMPLER_MODE_CLAMPTOEDGE);

	depth_texture = SDL_CreateGPUTexture(
		context.device,
		&(SDL_GPUTextureCreateInfo) {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.width = context.width,
			.height = context.height,
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
			.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET
		}
	);

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
