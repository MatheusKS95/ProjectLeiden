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

void SCR_CreateEffectBuffers(EffectBuffers *buffers)
{
	if(buffers == NULL)
	{
		return;
	}

	buffers->vbuffer = SDL_CreateGPUBuffer(
		drawing_context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = sizeof(EffectVertex) * 4
		}
	);

	buffers->ibuffer = SDL_CreateGPUBuffer(
		drawing_context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_INDEX,
			.size = sizeof(Uint32) * 6
		}
	);

	SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
		drawing_context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(EffectVertex) * 4) + (sizeof(Uint32) * 6)
		}
	);

	EffectVertex* transferData = SDL_MapGPUTransferBuffer(
		drawing_context.device,
		bufferTransferBuffer,
		false
	);

	transferData[0] = (EffectVertex) { -1,  1, 0, 0, 0 };
	transferData[1] = (EffectVertex) {  1,  1, 0, 1, 0 };
	transferData[2] = (EffectVertex) {  1, -1, 0, 1, 1 };
	transferData[3] = (EffectVertex) { -1, -1, 0, 0, 1 };

	Uint32* indexData = (Uint32*) &transferData[4];
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 0;
	indexData[4] = 2;
	indexData[5] = 3;

	SDL_UnmapGPUTransferBuffer(drawing_context.device, bufferTransferBuffer);

	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(drawing_context.device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = bufferTransferBuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = buffers->vbuffer,
			.offset = 0,
			.size = sizeof(EffectVertex) * 4
		},
		false
	);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = bufferTransferBuffer,
			.offset = sizeof(EffectVertex) * 4
		},
		&(SDL_GPUBufferRegion) {
			.buffer = buffers->ibuffer,
			.offset = 0,
			.size = sizeof(Uint32) * 6
		},
		false
	);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_ReleaseGPUTransferBuffer(drawing_context.device, bufferTransferBuffer);
}

void SCR_ReleaseEffectBuffers(EffectBuffers *buffers)
{
	if(buffers != NULL)
	{
		SDL_ReleaseGPUBuffer(drawing_context.device, buffers->vbuffer);
		SDL_ReleaseGPUBuffer(drawing_context.device, buffers->ibuffer);
	}
}

//this only handles a vertex buffer with position and UV
//useful for retro rendering - but not so much for more advanced NPR
SDL_GPUGraphicsPipeline *SCR_CreateSimplePipeline(SDL_GPUShader *vs,
													SDL_GPUShader *fs,
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