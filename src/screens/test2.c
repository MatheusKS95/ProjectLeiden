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
#include <shader.h>
#include <screens.h>

static SDL_GPUGraphicsPipeline *simple;
static SDL_GPUSampler *sampler;
static SDL_GPUTexture *depth_texture;
static Model *test_model;
static Matrix4x4 test_model_transform;

static float deltatime;
static float lastframe;
static float velocity;

static float last_x, last_y;
static float mouse_x, mouse_y;
static bool first_mouse;
static Camera cam_1;

bool TestScreen2_Setup()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting retro test screen...");

	int width, height;
	SDL_GetWindowSize(drawing_context.window, &width, &height);
	deltatime = lastframe = 0.0f;
	last_x = width / 2.0f;
	last_y = height / 2.0f;
	mouse_x = last_x;
	mouse_y = last_y;
	first_mouse = true;
	//need to bring lookat, or a camera update thing
	InitCameraBasic(&cam_1, (Vector3){0.0f, 1.3f, 8.0f}, (float)width / (float)height);

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
	simple = SCR_CreateSimplePipeline(vsimpleshader, fsimpleshader, true);

	test_model = (Model*)SDL_malloc(sizeof(Model));
	if(test_model != NULL)
	{
		ImportIQM(drawing_context.device, test_model, "testmodels/tower/tower.iqm");
	}

	SDL_GPUSamplerCreateInfo samplercreateinfo = { 0 };
	samplercreateinfo.min_filter = SDL_GPU_FILTER_NEAREST;
	samplercreateinfo.mag_filter = SDL_GPU_FILTER_NEAREST;
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

	return true;
}

void TestScreen2_Input(SDL_Event event)
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
				TestScreen2_Destroy();
				current_screen = SCREEN_SPLASH;
				return;
			}
		}
	}
}

void TestScreen2_Iterate()
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

	test_model_transform = Matrix4x4_Identity();
	test_model_transform = Matrix4x4_Rotate(test_model_transform, (Vector3){0.0f, 1.0f, 0.0f}, DegToRad(SDL_GetTicks() / 20));
	test_model_transform = Matrix4x4_Translate(test_model_transform, 0.0f, 0.0f, -8.0f);
}

void TestScreen2_Draw()
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
	colorTargetInfo.texture = swapchain_texture;
	colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	SDL_GPURenderPass *renderpass_simple = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthstenciltargetinfo);
	Matrix4x4 viewproj;
	viewproj = Matrix4x4_Mul(cam_1.view, cam_1.projection);
	//Matrix4x4 mvp = Matrix4x4_Mul(car->transform, viewproj);
	Matrix4x4 mvp = Matrix4x4_Mul(test_model_transform, viewproj);
	for(size_t i = 0; i < test_model->meshes.count; i++)
	{
		Mesh *mesh = &test_model->meshes.meshes[i];
		//binding graphics pipeline
		SDL_BindGPUGraphicsPipeline(renderpass_simple, simple);

		//binding vertex and index buffers
		SDL_BindGPUVertexBuffers(renderpass_simple, 0, &(SDL_GPUBufferBinding){ mesh->vbuffer, 0 }, 1);
		SDL_BindGPUIndexBuffer(renderpass_simple, &(SDL_GPUBufferBinding){ mesh->ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);

		SDL_BindGPUFragmentSamplers(renderpass_simple, 0, &(SDL_GPUTextureSamplerBinding){ mesh->diffuse.texture, sampler }, 1);

		//UBO
		SDL_PushGPUVertexUniformData(cmdbuf, 0, &mvp, sizeof(mvp));

		SDL_DrawGPUIndexedPrimitives(renderpass_simple, mesh->iarray.count, 1, 0, 0, 0);
	}
	SDL_EndGPURenderPass(renderpass_simple);

	SDL_SubmitGPUCommandBuffer(cmdbuf);
	return;
}

void TestScreen2_Destroy()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing test screen 2...");
	ReleaseModel(drawing_context.device, test_model);
	SDL_ReleaseGPUGraphicsPipeline(drawing_context.device, simple);
	SDL_ReleaseGPUSampler(drawing_context.device, sampler);
	SDL_ReleaseGPUTexture(drawing_context.device, depth_texture);
	return;
}
