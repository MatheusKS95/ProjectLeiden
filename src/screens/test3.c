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

static bool collision;

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
	renderstuff.pipeline = SCR_CreateSimplePipeline(vsimpleshader, fsimpleshader, true);

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
	tower.box.center = tower.position;
	tower.box.half_size = (Vector3){1.0f, 1.0f, 1.0f};

	//load box
	box = (test3object){ 0 };
	box.model = (Model*)SDL_malloc(sizeof(Model));
	if(box.model != NULL)
	{
		ImportIQM(drawing_context.device, box.model, "testmodels/cube/cube.iqm");
	}
	box.position = (Vector3){ 4.0f, 0.0f, 5.0f };
	box.scale = 1.0f;
	box.box.center = box.position;
	box.box.half_size = (Vector3){1.0f, 1.0f, 1.0f};

	collision = false;

	return true;
}

void TestScreen3_Input(SDL_Event event)
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
				return;
			}
		}
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

	return;
}

void TestScreen3_Iterate()
{
	float current_frame = (float)SDL_GetTicks();
	deltatime = current_frame - lastframe;
	lastframe = current_frame;

	box.box.center = box.position;

	if(Physics_AABBvsAABB(box.box, tower.box))
	{
		collision = true;
	}
	else
	{
		collision = false;
	}
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

	SDL_FColor clearcolor;
	if(collision)
		clearcolor = (SDL_FColor){ 0.4f, 0.0f, 0.0f, 1.0f };
	else
		clearcolor = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };

	SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
	colorTargetInfo.texture = swapchain_texture;
	colorTargetInfo.clear_color = clearcolor;
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
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing test screen 3...");
	ReleaseModel(drawing_context.device, tower.model);
	ReleaseModel(drawing_context.device, box.model);
	SDL_ReleaseGPUGraphicsPipeline(drawing_context.device, renderstuff.pipeline);
	SDL_ReleaseGPUSampler(drawing_context.device, renderstuff.sampler);
	SDL_ReleaseGPUTexture(drawing_context.device, renderstuff.depth_texture);
	return;
}
