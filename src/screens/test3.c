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
#include <assets.h>
#include <linmath.h>
#include <physics.h>
#include <screens.h>
#include <shader.h>
#include <list.h>

typedef struct test3object
{
	Model *model;
	AABB box;
	Vector3 position;
	float scale;
} test3object;

typedef struct test3render
{
	SDL_GPUGraphicsPipeline *pipeline;
	SDL_GPUSampler *sampler;
	SDL_GPUTexture *depth_texture;
} test3render;

static test3render renderstuff;

static test3object tower;
static test3object box;

static float deltatime;
static float lastframe;

static float last_x, last_y;
static float mouse_x, mouse_y;
static bool first_mouse;
static Camera cam_1;

static SDL_GPUGraphicsPipeline *createpipeline_retro(SDL_GPUShader *vs, SDL_GPUShader *fs,
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

bool TestScreen3_Setup()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting physics test screen...");

	renderstuff = (test3render){ 0 };

	int width, height;
	SDL_GetWindowSize(drawing_context.window, &width, &height);
	deltatime = lastframe = 0.0f;
	last_x = width / 2.0f;
	last_y = height / 2.0f;
	mouse_x = last_x;
	mouse_y = last_y;
	first_mouse = true;
	//need to bring lookat, or a camera update thing
	//InitCameraBasic(&cam_1, (Vector3){0.0f, 20.0f, 30.0f}, (float)width / (float)height);
	InitCameraFull(&cam_1, (Vector3){0.0f, 20.0f, 30.0f}, (Vector3){0.0f, 1.0f, 0.0f},
					-90.0f, -30.0f, 0.0f, 45.0f, (float)width / (float)height);

	SDL_GPUShader *vsimpleshader = LoadShader("shaders/fifthgen/fifthgen.vert.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	if(vsimpleshader == NULL)
	{
		SDL_Log("Failed to load simple vertex shader.");
		return NULL;
	}
	SDL_GPUShader *fsimpleshader = LoadShader("shaders/fifthgen/fifthgen.frag.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(fsimpleshader == NULL)
	{
		SDL_Log("Failed to load simple fragment shader.");
		return NULL;
	}
	renderstuff.pipeline = createpipeline_retro(vsimpleshader, fsimpleshader, true);

	SDL_GPUSamplerCreateInfo samplercreateinfo = { 0 };
	samplercreateinfo.min_filter = SDL_GPU_FILTER_NEAREST;
	samplercreateinfo.mag_filter = SDL_GPU_FILTER_NEAREST;
	samplercreateinfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	samplercreateinfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplercreateinfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplercreateinfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	renderstuff.sampler = SDL_CreateGPUSampler(drawing_context.device, &samplercreateinfo);

	renderstuff.depth_texture = SDL_CreateGPUTexture(
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

	//load tower
	tower = (test3object){ 0 };
	tower.model = (Model*)SDL_malloc(sizeof(Model));
	if(tower.model != NULL)
	{
		ImportIQM(drawing_context.device, tower.model, "testmodels/tower/tower.iqm");
	}
	tower.position = (Vector3){ 0 };
	tower.scale = 1.0f;
	tower.box = (AABB){ 0 };

	//load box
	box = (test3object){ 0 };
	box.model = (Model*)SDL_malloc(sizeof(Model));
	if(box.model != NULL)
	{
		ImportIQM(drawing_context.device, box.model, "testmodels/cube/cube.iqm");
	}
	box.position = (Vector3){ 2.0f, 0.0f, 1.0f };
	box.scale = 1.0f;
	box.box = (AABB){ 0 };

	return true;
}

void TestScreen3_Logic(SDL_Event event)
{
	float current_frame = (float)SDL_GetTicks();
	deltatime = current_frame - lastframe;
	lastframe = current_frame;
	//float velocity = 0.01f * deltatime;
	//TODO mouse
	if(first_mouse)
	{
		last_x = mouse_x;
		last_y = mouse_y;
		first_mouse = false;
	}
	while(SDL_PollEvent(&event))
	{
		if(event.type == SDL_EVENT_QUIT)
		{
			exit_signal = true;
		}
		if(event.type == SDL_EVENT_KEY_DOWN)
		{
			if(event.key.key == SDLK_ESCAPE)
			{
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "leaving...");
				if(SplashScreen_Setup())
				{
					TestScreen3_Destroy();
					current_screen = SCREEN_SPLASH;
					break;
				}
			}
			//TODO check and update collisions
			if(event.key.key == SDLK_LEFT)
			{
				box.position.x -= 0.5f;
			}
			if(event.key.key == SDLK_RIGHT)
			{
				box.position.x += 0.5f;
			}
			if(event.key.key == SDLK_UP)
			{
				box.position.z -= 0.5f;
			}
			if(event.key.key == SDLK_DOWN)
			{
				box.position.z += 0.5f;
			}
		}
		/*if(event.type == SDL_EVENT_MOUSE_MOTION)
		{
			//mouse_x = state->mouse_x;
			//mouse_y = state->mouse_y;
			float x_offset = mouse_x + last_x;
			float y_offset = last_y + mouse_y;
			last_x = mouse_x;
			last_y = mouse_y;
			TestCameraFreecam(&cam_1, x_offset, y_offset, true);
			mouse_x = mouse_y = 0;
		}*/
	}

	return;
}

static void drawobject(test3object *object, SDL_GPURenderPass *renderpass, SDL_GPUCommandBuffer *cmdbuf, SDL_GPUGraphicsPipeline *pipeline)
{
	Matrix4x4 viewproj;
	viewproj = Matrix4x4_Mul(cam_1.view, cam_1.projection);

	Matrix4x4 model = Matrix4x4_Identity();
	model = Matrix4x4_Scale(model, (Vector3){object->scale, object->scale, object->scale});
	model = Matrix4x4_Translate(model, object->position.x, object->position.y, object->position.z);

	Matrix4x4 mvp = Matrix4x4_Mul(model, viewproj);
	for(size_t i = 0; i < object->model->meshes.count; i++)
	{
		Mesh *mesh = &object->model->meshes.meshes[i];
		//binding graphics pipeline
		SDL_BindGPUGraphicsPipeline(renderpass, pipeline);

		//binding vertex and index buffers
		SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding){ mesh->vbuffer, 0 }, 1);
		SDL_BindGPUIndexBuffer(renderpass, &(SDL_GPUBufferBinding){ mesh->ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);

		SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding){ mesh->diffuse.texture, renderstuff.sampler }, 1);

		//UBO
		SDL_PushGPUVertexUniformData(cmdbuf, 0, &mvp, sizeof(mvp));

		SDL_DrawGPUIndexedPrimitives(renderpass, mesh->iarray.count, 1, 0, 0, 0);
	}
}

void TestScreen3_Draw()
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
	depthstenciltargetinfo.texture = renderstuff.depth_texture;
	depthstenciltargetinfo.cycle = true;
	depthstenciltargetinfo.clear_depth = 1;
	depthstenciltargetinfo.clear_stencil = 0;
	depthstenciltargetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
	depthstenciltargetinfo.store_op = SDL_GPU_STOREOP_STORE;
	depthstenciltargetinfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
	depthstenciltargetinfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
	colorTargetInfo.texture = swapchain_texture;
	colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	SDL_GPURenderPass *renderpass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthstenciltargetinfo);

	drawobject(&tower, renderpass, cmdbuf, renderstuff.pipeline);
	drawobject(&box, renderpass, cmdbuf, renderstuff.pipeline);

	SDL_EndGPURenderPass(renderpass);

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void TestScreen3_Destroy()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing test screen 2...");
	ReleaseModel(drawing_context.device, tower.model);
	ReleaseModel(drawing_context.device, box.model);
	SDL_ReleaseGPUGraphicsPipeline(drawing_context.device, renderstuff.pipeline);
	SDL_ReleaseGPUSampler(drawing_context.device, renderstuff.sampler);
	SDL_ReleaseGPUTexture(drawing_context.device, renderstuff.depth_texture);
	return;
}
