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

//will be retired

#include <SDL3/SDL.h>
#include <fileio.h>
#include <assets.h>
#include <shader.h>
#include <screens.h>

static SDL_GPUGraphicsPipeline *effect_pipeline;
static SDL_GPUSampler *effect_sampler;
static EffectBuffers effect_buffer;

static SDL_GPUGraphicsPipeline *norm_pipeline;
static SDL_GPUTexture *scene_normtexture;

static SDL_GPUTexture *scene_colortexture;

static SDL_GPUGraphicsPipeline *simple;
static SDL_GPUSampler *sampler;
static SDL_GPUTexture *depth_texture;
static Model *car;
static Matrix4x4 car_transform;

static float deltatime;
static float lastframe;
static float velocity;

static float last_x, last_y;
static float mouse_x, mouse_y;
static bool first_mouse;
static Camera cam_1;

static SDL_GPUGraphicsPipeline *createpipeline_simple(SDL_GPUShader *vs, SDL_GPUShader *fs,
														bool release_shaders)
{
	if(vs == NULL || fs == NULL)
	{
		return NULL;
	}

	SDL_GPUGraphicsPipelineCreateInfo pipeline_createinfo = { 0 };
	pipeline_createinfo = (SDL_GPUGraphicsPipelineCreateInfo)
	{
		.target_info =
		{
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
				.format = SDL_GetGPUSwapchainTextureFormat(drawing_context.device, drawing_context.window)
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
				.pitch = sizeof(Vertex3D)
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
			}} //there's more, but I need only these now
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vs,
		.fragment_shader = fs
	};
	SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(drawing_context.device, &pipeline_createinfo);
	if(release_shaders)
	{
		SDL_ReleaseGPUShader(drawing_context.device, vs);
		SDL_ReleaseGPUShader(drawing_context.device, fs);
	}

	return pipeline;
}

static SDL_GPUGraphicsPipeline *createpipeline_norm(SDL_GPUShader *vs, SDL_GPUShader *fs,
														bool release_shaders)
{
	if(vs == NULL || fs == NULL)
	{
		return NULL;
	}

	SDL_GPUGraphicsPipelineCreateInfo pipeline_createinfo = { 0 };
	pipeline_createinfo = (SDL_GPUGraphicsPipelineCreateInfo)
	{
		.target_info =
		{
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
				.format = SDL_GetGPUSwapchainTextureFormat(drawing_context.device, drawing_context.window)
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
				.pitch = sizeof(Vertex3D)
			}},
			.num_vertex_attributes = 2,
			.vertex_attributes = (SDL_GPUVertexAttribute[]){{
				//position
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 0,
				.offset = 0
			}, {
				//normal
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 1,
				.offset = (sizeof(float) * 3)
			}} //there's more, but I need only these now
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vs,
		.fragment_shader = fs
	};
	SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(drawing_context.device, &pipeline_createinfo);
	if(release_shaders)
	{
		SDL_ReleaseGPUShader(drawing_context.device, vs);
		SDL_ReleaseGPUShader(drawing_context.device, fs);
	}

	return pipeline;
}

static SDL_GPUGraphicsPipeline *createpipeline_effect(SDL_GPUShader *vs, SDL_GPUShader *fs,
														bool release_shaders)
{
	if(vs == NULL || fs == NULL)
	{
		return NULL;
	}

	SDL_GPUGraphicsPipelineCreateInfo pipeline_createinfo = { 0 };
	pipeline_createinfo = (SDL_GPUGraphicsPipelineCreateInfo)
	{
		.target_info =
		{
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
				.format = SDL_GetGPUSwapchainTextureFormat(drawing_context.device, drawing_context.window)
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
				.pitch = sizeof(EffectVertex)
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
		.vertex_shader = vs,
		.fragment_shader = fs
	};
	SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(drawing_context.device, &pipeline_createinfo);
	if(release_shaders)
	{
		SDL_ReleaseGPUShader(drawing_context.device, vs);
		SDL_ReleaseGPUShader(drawing_context.device, fs);
	}

	return pipeline;
}

bool TestScreen1_Setup()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting simple test screen...");

	int width, height;
	SDL_GetWindowSize(drawing_context.window, &width, &height);
	deltatime = lastframe = 0.0f;
	last_x = width / 2.0f;
	last_y = height / 2.0f;
	mouse_x = last_x;
	mouse_y = last_y;
	first_mouse = true;
	//need to bring lookat, or a camera update thing
	InitCameraBasic(&cam_1, (Vector3){0.0f, 0.0f, 8.0f}, (float)width / (float)height);

	SDL_GPUShader *vsimpleshader = LoadShader("shaders/simpletest/simple.vert.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	if(vsimpleshader == NULL)
	{
		SDL_Log("Failed to load simple vertex shader.");
		return NULL;
	}
	SDL_GPUShader *fsimpleshader = LoadShader("shaders/simpletest/simple.frag.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(fsimpleshader == NULL)
	{
		SDL_Log("Failed to load simple fragment shader.");
		return NULL;
	}
	simple = createpipeline_simple(vsimpleshader, fsimpleshader, true);

	SDL_GPUShader *vnormshader = LoadShader("shaders/norm/norm.vert.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	if(vsimpleshader == NULL)
	{
		SDL_Log("Failed to load norm vertex shader.");
		return NULL;
	}
	SDL_GPUShader *fnormshader = LoadShader("shaders/norm/norm.frag.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0, 0, 0);
	if(fnormshader == NULL)
	{
		SDL_Log("Failed to load skybox fragment shader.");
		return NULL;
	}
	norm_pipeline = createpipeline_simple(vnormshader, fnormshader, true);

	car = (Model*)SDL_malloc(sizeof(Model));
	if(car != NULL)
	{
		ImportIQM(drawing_context.device, car, "testmodels/nimrud/nimrud_body.iqm");
	}

	SDL_GPUSamplerCreateInfo samplercreateinfo = { 0 };
	samplercreateinfo.min_filter = SDL_GPU_FILTER_LINEAR;
	samplercreateinfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	samplercreateinfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	samplercreateinfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplercreateinfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplercreateinfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sampler = SDL_CreateGPUSampler(drawing_context.device, &samplercreateinfo);

	depth_texture = SDL_CreateGPUTexture(
		drawing_context.device,
		&(SDL_GPUTextureCreateInfo) {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.width = width,
			.height = height,
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
			.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET
		}
	);

	scene_colortexture = SDL_CreateGPUTexture(
		drawing_context.device,
		&(SDL_GPUTextureCreateInfo) {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.width = width,
			.height = height,
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
			.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET
		}
	);

	scene_normtexture = SDL_CreateGPUTexture(
		drawing_context.device,
		&(SDL_GPUTextureCreateInfo) {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.width = width,
			.height = height,
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
			.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET
		}
	);

	//effect stuff
	SDL_GPUShader *effectvs = LoadShader("shaders/outline/default.vert.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 0);
	if(effectvs == NULL)
	{
		SDL_Log("Failed to load effect vertex shader.");
		return NULL;
	}
	//tried with blur, some sharpening, etc. all did work
	SDL_GPUShader *effectfs = LoadShader("shaders/outline/outline.frag.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_FRAGMENT, 2, 1, 0, 0);
	if(effectfs == NULL)
	{
		SDL_Log("Failed to load effect fragment shader.");
		return NULL;
	}
	effect_pipeline = createpipeline_effect(effectvs, effectfs, true);

	samplercreateinfo = (SDL_GPUSamplerCreateInfo){ 0 };
	samplercreateinfo.min_filter = SDL_GPU_FILTER_NEAREST;
	samplercreateinfo.mag_filter = SDL_GPU_FILTER_NEAREST;
	samplercreateinfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	samplercreateinfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	samplercreateinfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	samplercreateinfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	effect_sampler = SDL_CreateGPUSampler(drawing_context.device, &samplercreateinfo);

	//TODO gen buffers
	effect_buffer = (EffectBuffers){ 0 };
	SCR_CreateEffectBuffers(&effect_buffer);

	return true;
}

void TestScreen1_Input(SDL_Event event)
{
	if(event.type == SDL_EVENT_QUIT)
	{
		exit_signal = true;
	}
	if(event.type == SDL_EVENT_KEY_DOWN)
	{
		if(event.key.key == SDLK_RIGHT)
		{
			Vector3 aux = { 0 };
			aux.x = cam_1.right.x * velocity;
			aux.y = cam_1.right.y * velocity;
			aux.z = cam_1.right.z * velocity;
			UpdateCameraPosition(&cam_1, Vector3_Add(cam_1.position, aux));
		}
		if(event.key.key == SDLK_LEFT)
		{
			Vector3 aux = { 0 };
			aux.x = cam_1.right.x * velocity;
			aux.y = cam_1.right.y * velocity;
			aux.z = cam_1.right.z * velocity;
			Vector3 newpos = cam_1.position;
			newpos.x = newpos.x - aux.x;
			newpos.y = newpos.y - aux.y;
			newpos.z = newpos.z - aux.z;
			UpdateCameraPosition(&cam_1, newpos);
		}
		if(event.key.key == SDLK_ESCAPE)
		{
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "leaving...");
			if(SplashScreen_Setup())
			{
				TestScreen1_Destroy();
				current_screen = SCREEN_SPLASH;
				return;
			}
		}
	}

	return;
}

void TestScreen1_Iterate()
{
	float current_frame = (float)SDL_GetTicks();
	deltatime = current_frame - lastframe;
	lastframe = current_frame;
	velocity = 0.01f * deltatime;

	//TODO mouse
	if(first_mouse)
	{
		last_x = mouse_x;
		last_y = mouse_y;
		first_mouse = false;
	}

	car_transform = Matrix4x4_Identity();
	//car_transform = Matrix4x4_Scale(car_transform, (Vector3){0.1f, 0.1f, 0.1f});
	car_transform = Matrix4x4_Rotate(car_transform, (Vector3){0.0f, 1.0f, 0.0f}, DegToRad(SDL_GetTicks() / 20));
	car_transform = Matrix4x4_Translate(car_transform, 0.0f, 0.0f, -8.0f);
}

void TestScreen1_Draw()
{
	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(drawing_context.device);
	if (cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to acquire command buffer.");
		return;
	}

	SDL_GPUTexture* swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, drawing_context.window, &swapchain_texture, NULL, NULL))
	{
		return ;
	}

	if(swapchain_texture == NULL)
	{
		return;
	}

	SDL_GPUDepthStencilTargetInfo depthstenciltargetinfo = { 0 };
	depthstenciltargetinfo.texture = depth_texture;
	depthstenciltargetinfo.cycle = true;
	depthstenciltargetinfo.clear_depth = 1;
	depthstenciltargetinfo.clear_stencil = 0;
	depthstenciltargetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
	depthstenciltargetinfo.store_op = SDL_GPU_STOREOP_STORE;
	depthstenciltargetinfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
	depthstenciltargetinfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

	//SIMPLE RENDER PASS
	SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
	colorTargetInfo.texture = scene_colortexture;
	colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.7f, 0.5f, 1.0f };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	SDL_GPURenderPass *renderpass_simple = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthstenciltargetinfo);
	Matrix4x4 viewproj;
	viewproj = Matrix4x4_Mul(cam_1.view, cam_1.projection);
	//Matrix4x4 mvp = Matrix4x4_Mul(car->transform, viewproj);
	Matrix4x4 mvp = Matrix4x4_Mul(car_transform, viewproj);
	for(size_t i = 0; i < car->meshes.count; i++)
	{
		Mesh *mesh = &car->meshes.meshes[i];
		//binding graphics pipeline
		SDL_BindGPUGraphicsPipeline(renderpass_simple, simple);

		//binding vertex and index buffers
		SDL_BindGPUVertexBuffers(renderpass_simple, 0, &(SDL_GPUBufferBinding){ mesh->vbuffer, 0 }, 1);
		SDL_BindGPUIndexBuffer(renderpass_simple, &(SDL_GPUBufferBinding){ mesh->ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);

		//texture samplers
		/*if(mesh->diffuse != NULL)
		{
			SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding){ mesh->diffuse->texture, sampler }, 1);
		}*/
		SDL_BindGPUFragmentSamplers(renderpass_simple, 0, &(SDL_GPUTextureSamplerBinding){ mesh->diffuse.texture, sampler }, 1);

		//UBO
		SDL_PushGPUVertexUniformData(cmdbuf, 0, &mvp, sizeof(mvp));

		SDL_DrawGPUIndexedPrimitives(renderpass_simple, mesh->iarray.count, 1, 0, 0, 0);
	}
	SDL_EndGPURenderPass(renderpass_simple);

	//NORM RENDER PASS
	colorTargetInfo = (SDL_GPUColorTargetInfo){ 0 };
	colorTargetInfo.texture = scene_normtexture;
	colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.7f, 0.5f, 1.0f };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	SDL_GPURenderPass *renderpass_norm = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthstenciltargetinfo);
	for(size_t i = 0; i < car->meshes.count; i++)
	{
		Mesh *mesh = &car->meshes.meshes[i];
		//binding graphics pipeline
		SDL_BindGPUGraphicsPipeline(renderpass_norm, norm_pipeline);

		//binding vertex and index buffers
		SDL_BindGPUVertexBuffers(renderpass_norm, 0, &(SDL_GPUBufferBinding){ mesh->vbuffer, 0 }, 1);
		SDL_BindGPUIndexBuffer(renderpass_norm, &(SDL_GPUBufferBinding){ mesh->ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);

		//UBO
		struct ubo
		{
			Matrix4x4 mvp;
			Matrix4x4 matmodel;
		};
		struct ubo ubo_object = {mvp, car_transform};
		SDL_PushGPUVertexUniformData(cmdbuf, 0, &ubo_object, sizeof(ubo_object));

		SDL_DrawGPUIndexedPrimitives(renderpass_norm, mesh->iarray.count, 1, 0, 0, 0);
	}
	SDL_EndGPURenderPass(renderpass_norm);

	//EFFECT RENDER PASS
	colorTargetInfo = (SDL_GPUColorTargetInfo){ 0 };
	colorTargetInfo.texture = swapchain_texture;
	colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

	int width, height;
	SDL_GetWindowSize(drawing_context.window, &width, &height);
	Vector2 screensize = {(float)width, (float)height};

	SDL_GPURenderPass *renderpass_effect = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
	SDL_PushGPUFragmentUniformData(cmdbuf, 0, &screensize, sizeof(screensize));
	SDL_BindGPUGraphicsPipeline(renderpass_effect, effect_pipeline);
	SDL_BindGPUVertexBuffers(renderpass_effect, 0, &(SDL_GPUBufferBinding){ effect_buffer.vbuffer, 0 }, 1);
	SDL_BindGPUIndexBuffer(renderpass_effect, &(SDL_GPUBufferBinding){ effect_buffer.ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);
	SDL_BindGPUFragmentSamplers(renderpass_effect, 0, (SDL_GPUTextureSamplerBinding[]){
									{ .texture = scene_normtexture, .sampler = effect_sampler },
									{ .texture = scene_colortexture,.sampler = effect_sampler }}, 2);
	SDL_DrawGPUIndexedPrimitives(renderpass_effect, 6, 1, 0, 0, 0);
	SDL_EndGPURenderPass(renderpass_effect);

	SDL_SubmitGPUCommandBuffer(cmdbuf);
	return;
}

void TestScreen1_Destroy()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing test screen 1...");
	ReleaseModel(drawing_context.device, car);
	SCR_ReleaseEffectBuffers(&effect_buffer);
	SDL_ReleaseGPUGraphicsPipeline(drawing_context.device, effect_pipeline);
	SDL_ReleaseGPUGraphicsPipeline(drawing_context.device, norm_pipeline);
	SDL_ReleaseGPUGraphicsPipeline(drawing_context.device, simple);
	SDL_ReleaseGPUSampler(drawing_context.device, effect_sampler);
	SDL_ReleaseGPUSampler(drawing_context.device, sampler);
	SDL_ReleaseGPUTexture(drawing_context.device, scene_normtexture);
	SDL_ReleaseGPUTexture(drawing_context.device, scene_colortexture);
	SDL_ReleaseGPUTexture(drawing_context.device, depth_texture);
	return;
}
