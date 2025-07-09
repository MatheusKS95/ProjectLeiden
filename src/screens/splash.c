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
#include <fileio.h>
#include <assets.h>
#include <shader.h>
#include <screens.h>

typedef struct Quad
{
	float x, y, z;
	float u, v;
} Quad;

static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUBuffer* vbuffer;
static SDL_GPUBuffer* ibuffer;
static Texture2D texture;
static SDL_GPUSampler* sampler;

bool SplashScreen_Setup()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting splash screen...");

	SDL_GPUShader *vs = LoadShader("shaders/splash/quad.vert.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 0);
	if(vs == NULL)
	{
		return false;
	}
	SDL_GPUShader *fs = LoadShader("shaders/splash/quad.frag.spv", drawing_context.device, SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(fs == NULL)
	{
		return false;
	}

	if(!LoadTextureFile(drawing_context.device, &texture, "splash/splash2.qoi"))
	{
		return false;
	}

	// Create the pipeline
	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.target_info = {
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
				.format = SDL_GetGPUSwapchainTextureFormat(drawing_context.device, drawing_context.window)
			}},
		},
		.vertex_input_state = (SDL_GPUVertexInputState){
			.num_vertex_buffers = 1,
			.vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{
				.slot = 0,
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0,
				.pitch = sizeof(Quad)
			}},
			.num_vertex_attributes = 2,
			.vertex_attributes = (SDL_GPUVertexAttribute[]){{
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 0,
				.offset = 0
			}, {
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
				.location = 1,
				.offset = sizeof(float) * 3
			}}
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vs,
		.fragment_shader = fs
	};

	pipeline = SDL_CreateGPUGraphicsPipeline(drawing_context.device, &pipelineCreateInfo);
	if (pipeline == NULL)
	{
		SDL_Log("Failed to create pipeline!");
		return -1;
	}

	SDL_ReleaseGPUShader(drawing_context.device, vs);
	SDL_ReleaseGPUShader(drawing_context.device, fs);
	sampler = SDL_CreateGPUSampler(drawing_context.device, &(SDL_GPUSamplerCreateInfo){
		.min_filter = SDL_GPU_FILTER_NEAREST,
		.mag_filter = SDL_GPU_FILTER_NEAREST,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
	});

	vbuffer = SDL_CreateGPUBuffer(
		drawing_context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = sizeof(Quad) * 4
		}
	);

	ibuffer = SDL_CreateGPUBuffer(
		drawing_context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_INDEX,
			.size = sizeof(Uint32) * 6
		}
	);

	// Set up buffer data
	SDL_GPUTransferBuffer* buffer_transferbuffer = SDL_CreateGPUTransferBuffer(
		drawing_context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(Quad) * 4) + (sizeof(Uint32) * 6)
		}
	);

	Quad* transferdata = SDL_MapGPUTransferBuffer(
		drawing_context.device,
		buffer_transferbuffer,
		false
	);

	transferdata[0] = (Quad) { -1,  1, 0, 0, 0 };
	transferdata[1] = (Quad) {  1,  1, 0, 1, 0 };
	transferdata[2] = (Quad) {  1, -1, 0, 1, 1 };
	transferdata[3] = (Quad) { -1, -1, 0, 0, 1 };

	Uint32* indexdata = (Uint32*) &transferdata[4];
	indexdata[0] = 0;
	indexdata[1] = 1;
	indexdata[2] = 2;
	indexdata[3] = 0;
	indexdata[4] = 2;
	indexdata[5] = 3;

	SDL_UnmapGPUTransferBuffer(drawing_context.device, buffer_transferbuffer);

	// Upload the transfer data to the GPU resources
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(drawing_context.device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = buffer_transferbuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = vbuffer,
			.offset = 0,
			.size = sizeof(Quad) * 4
		},
		false
	);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = buffer_transferbuffer,
			.offset = sizeof(Quad) * 4
		},
		&(SDL_GPUBufferRegion) {
			.buffer = ibuffer,
			.offset = 0,
			.size = sizeof(Uint32) * 6
		},
		false
	);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_ReleaseGPUTransferBuffer(drawing_context.device, buffer_transferbuffer);
	return true;
}

void SplashScreen_Logic(SDL_Event event)
{
	//TODO check keys
	while(SDL_PollEvent(&event))
	{
		if(event.type == SDL_EVENT_QUIT)
		{
			exit_signal = true;
		}
		if(event.type == SDL_EVENT_KEY_DOWN)
		{
			if(event.key.key != SDLK_Q)
			{
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "going to cel shading test");
				if(TestScreen1_Setup())
				{
					SplashScreen_Destroy();
					current_screen = SCREEN_TEST;
				}
			}
			else
			{
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "going to physics test");
				if(TestScreen2_Setup())
				{
					SplashScreen_Destroy();
					current_screen = SCREEN_TEST2;
				}
			}
		}
	}
	return;
}

void SplashScreen_Draw()
{
	SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(drawing_context.device);
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Splash screen: failed to acquire command buffer: %s", SDL_GetError());
		return;
	}
	SDL_GPUTexture *swapchain_texture;
	if(!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, drawing_context.window, &swapchain_texture, NULL, NULL))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Renderer: Failed to acquire swapchain texture: %s", SDL_GetError());
		return;
	}
	if(swapchain_texture != NULL)
	{
		SDL_GPUColorTargetInfo colortargetinfo = { 0 };
		colortargetinfo.texture = swapchain_texture;
		colortargetinfo.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
		colortargetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colortargetinfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colortargetinfo, 1, NULL);
		SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
		SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){ .buffer = vbuffer, .offset = 0 }, 1);
		SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){ .buffer = ibuffer, .offset = 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);
		SDL_BindGPUFragmentSamplers(renderPass, 0, &(SDL_GPUTextureSamplerBinding){ .texture = texture.texture, .sampler = sampler }, 1);
		SDL_DrawGPUIndexedPrimitives(renderPass, 6, 1, 0, 0, 0);
		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
	return;
}

void SplashScreen_Destroy()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing splash screen...");
	SDL_ReleaseGPUGraphicsPipeline(drawing_context.device, pipeline);
	SDL_ReleaseGPUBuffer(drawing_context.device, vbuffer);
	SDL_ReleaseGPUBuffer(drawing_context.device, ibuffer);
	ReleaseTexture2D(drawing_context.device, &texture);
	SDL_ReleaseGPUSampler(drawing_context.device, sampler);
	return;
}
