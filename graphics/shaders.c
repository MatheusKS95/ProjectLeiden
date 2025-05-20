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

bool Graphics_LoadShaderFromMem(Shader *shader,
								uint8_t *buffer, size_t size,
								const char *entrypoint,
								ShaderStage stage,
								Uint32 samplerCount,
								Uint32 uniformBufferCount,
								Uint32 storageBufferCount,
								Uint32 storageTextureCount)
{
	if(shader == NULL || buffer == NULL || size <= 0)
	{
		return false;
	}
	SDL_GPUShaderStage sdl_stage;
	switch(stage)
	{
		case SHADERSTAGE_VERTEX: sdl_stage = SDL_GPU_SHADERSTAGE_VERTEX; break;
		case SHADERSTAGE_FRAGMENT: sdl_stage = SDL_GPU_SHADERSTAGE_FRAGMENT; break;
		default: return false;
	}

	SDL_GPUShaderCreateInfo shader_info = {
		.code = buffer,
		.code_size = size,
		.entrypoint = entrypoint,
		.format = SDL_GPU_SHADERFORMAT_SPIRV, //the only one accepted for now
		.stage = sdl_stage,
		.num_samplers = samplerCount,
		.num_uniform_buffers = uniformBufferCount,
		.num_storage_buffers = storageBufferCount,
		.num_storage_textures = storageTextureCount
	};
	shader->shader = SDL_CreateGPUShader(context.device, &shader_info);
	if(shader->shader == NULL)
	{
		return false;
	}
	return true;
}

bool Graphics_LoadShaderFromFS(Shader *shader,
								const char *path,
								const char *entrypoint,
								ShaderStage stage,
								Uint32 samplerCount,
								Uint32 uniformBufferCount,
								Uint32 storageBufferCount,
								Uint32 storageTextureCount)
{
	if(shader == NULL || path == NULL) //TODO compare with incompatible strings
	{
		return false;
	}
	bool result = false;
	size_t filesize;
	Uint8 *file = FileIOReadBytes(path, &filesize);

	result = Graphics_LoadShaderFromMem(shader, file, filesize, entrypoint, stage,
										samplerCount, uniformBufferCount, storageBufferCount, storageTextureCount);
	return result;
}

bool Graphics_CreatePipelineSkybox(const char *path_vs,
										const char *path_fs)
{
	Shader vsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&vsshader, path_vs, "main", SHADERSTAGE_VERTEX, 0, 1, 0, 0))
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return false;
	}
	Shader fsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&fsshader, path_fs, "main", SHADERSTAGE_FRAGMENT, 1, 0, 0, 0))
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
			}}
		},
		.depth_stencil_state = (SDL_GPUDepthStencilState) {
			.enable_depth_test = true,
			.enable_depth_write = true,
			.enable_stencil_test = false,
			.compare_op = SDL_GPU_COMPAREOP_NEVER,
			.write_mask = 0xFF
		},
		.vertex_input_state = (SDL_GPUVertexInputState){
			.num_vertex_buffers = 1,
			.vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{
				.slot = 0,
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0,
				.pitch = sizeof(Vector3)
			}},
			.num_vertex_attributes = 1,
			.vertex_attributes = (SDL_GPUVertexAttribute[]){{
				//position
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 0,
				.offset = 0
			}}
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vsshader.shader,
		.fragment_shader = fsshader.shader
	};
	pipelines.skybox = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	SDL_ReleaseGPUShader(context.device, vsshader.shader);
	SDL_ReleaseGPUShader(context.device, fsshader.shader);

	return true;
}

bool Graphics_CreatePipelineSimple(const char *path_vs,
									const char *path_fs)
{
	Shader vsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&vsshader, path_vs, "main", SHADERSTAGE_VERTEX, 0, 1, 0, 0))
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return false;
	}
	Shader fsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&fsshader, path_fs, "main", SHADERSTAGE_FRAGMENT, 1, 0, 0, 0))
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
			}}
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vsshader.shader,
		.fragment_shader = fsshader.shader
	};
	pipelines.simple = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	SDL_ReleaseGPUShader(context.device, vsshader.shader);
	SDL_ReleaseGPUShader(context.device, fsshader.shader);

	return true;
}