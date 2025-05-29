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

/**
 * @file shaders.c
 * @brief Implementation file for shader stuff (not pipelines)
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/
#include <SDL3/SDL.h>
#include <fileio.h>
#include <graphics.h>

SDL_GPUShader* Graphics_LoadShader(const char *path,
									SDL_GPUShaderStage stage,
									Uint32 samplerCount,
									Uint32 uniformBufferCount,
									Uint32 storageBufferCount,
									Uint32 storageTextureCount)
{
	if(path == NULL || SDL_strcmp(path, "") == 0)
	{
		return NULL;
	}
	size_t filesize;
	Uint8 *file = FileIOReadBytes(path, &filesize);

	if(file == NULL)
	{
		//error message
		return NULL;
	}

	SDL_GPUShaderCreateInfo shader_info = {
		.code = file,
		.code_size = filesize,
		.entrypoint = "main",
		.format = SDL_GPU_SHADERFORMAT_SPIRV, //the only one accepted for now
		.stage = stage,
		.num_samplers = samplerCount,
		.num_uniform_buffers = uniformBufferCount,
		.num_storage_buffers = storageBufferCount,
		.num_storage_textures = storageTextureCount
	};
	return SDL_CreateGPUShader(context.device, &shader_info);
}

bool Graphics_CreateAndUploadStorageBuffer(StorageBuffer *buffer,
									void *data, size_t size)
{
	if(buffer == NULL || data == NULL || size <= 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't generate storage buffer.");
		return false;
	}

	buffer = SDL_CreateGPUBuffer(
		context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
			.size = size
		}
	);
	SDL_GPUTransferBuffer* transferbuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = size
		}
	);
	void* buffer_transferdata = SDL_MapGPUTransferBuffer(context.device, transferbuffer, false);
	SDL_memcpy(buffer_transferdata, data, size);
	SDL_UnmapGPUTransferBuffer(context.device, transferbuffer);

	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = transferbuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = buffer,
			.offset = 0,
			.size = size
		},
		false
	);

	SDL_EndGPUCopyPass(copyPass);
	SDL_ReleaseGPUTransferBuffer(context.device, transferbuffer);
	SDL_SubmitGPUCommandBuffer(cmdbuf);

	return true;
}

void Graphics_ReleaseStorageBuffer(StorageBuffer *buffer)
{
	if(buffer == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't release invalid storage buffer.");
		return;
	}
	SDL_ReleaseGPUBuffer(context.device, buffer);
}

//keeping this for future use
/*bool Graphics_CreatePipelineToon(const char *path_norm_vs,
									const char *path_norm_fs,
									const char *path_outl_vs,
									const char *path_outl_fs,
									const char *path_toon_vs,
									const char *path_toon_fs)
{
	SDL_GPUShader *norm_vsshader = Graphics_LoadShader(path_norm_vs, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	if(norm_vsshader == NULL)
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return false;
	}
	SDL_GPUShader *norm_fsshader = Graphics_LoadShader(path_norm_fs, SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(norm_fsshader == NULL)
	{
		SDL_Log("Failed to load skybox fragment shader.");
		return false;
	}
	SDL_GPUShader *outl_vsshader = Graphics_LoadShader(path_outl_vs, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	if(norm_vsshader == NULL)
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return false;
	}
	SDL_GPUShader *outl_fsshader = Graphics_LoadShader(path_outl_fs, SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(norm_fsshader == NULL)
	{
		SDL_Log("Failed to load skybox fragment shader.");
		return false;
	}
	//didn't complete the shaders for this one, need to complete it
	SDL_GPUShader *toon_vsshader = Graphics_LoadShader(path_toon_vs, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	if(toon_vsshader == NULL)
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return false;
	}
	SDL_GPUShader *toon_fsshader = Graphics_LoadShader(path_toon_fs, SDL_GPU_SHADERSTAGE_FRAGMENT, 4, 1, 1, 0);
	if(toon_fsshader == NULL)
	{
		SDL_Log("Failed to load skybox fragment shader.");
		return false;
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
			}, {
				//normal
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 2,
				.offset = (sizeof(float) * 3) + (sizeof(float) * 2)
			}, {
				//tangent
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
				.location = 3,
				.offset = (sizeof(float) * 3) + (sizeof(float) * 2) + (sizeof(float) * 3)
			}, {
				//color
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
				.location = 4,
				.offset = (sizeof(float) * 3) + (sizeof(float) * 2) + (sizeof(float) * 3) + (sizeof(float) * 4)
			//TODO there are more stuff (blend indices and weights)
			}}
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = norm_vsshader,
		.fragment_shader = norm_fsshader
	};
	pipelines.norm = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	SDL_ReleaseGPUShader(context.device, norm_vsshader);
	SDL_ReleaseGPUShader(context.device, norm_fsshader);

	pipeline_createinfo.vertex_shader = outl_vsshader;
	pipeline_createinfo.fragment_shader = outl_fsshader;
	pipelines.outline = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	SDL_ReleaseGPUShader(context.device, outl_vsshader);
	SDL_ReleaseGPUShader(context.device, outl_fsshader);

	pipeline_createinfo.vertex_shader = toon_vsshader;
	pipeline_createinfo.fragment_shader = toon_fsshader;
	pipelines.toon = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	SDL_ReleaseGPUShader(context.device, toon_vsshader);
	SDL_ReleaseGPUShader(context.device, toon_fsshader);

	return true;
}*/