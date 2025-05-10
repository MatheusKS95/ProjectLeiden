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

GeneralPipelines pipelines;

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

Pipeline Graphics_CreatePipeline(Shader *vs, Shader *fs,
									PipelineType type,
									bool release_shader)
{
	SDL_GPUGraphicsPipelineCreateInfo pipeline_createinfo = { 0 };
	if(type == PIPELINETYPE_3D)
	{
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
				.num_vertex_attributes = 5,
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
				//TODO there are more stuff to load (probably)
				}}
			},
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.vertex_shader = vs->shader,
			.fragment_shader = fs->shader
		};
	}
	//TODO other kind of pipelines (framebuffer/render to texture next)
	else
	{
		return NULL;
	}

	Pipeline pipeline = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	if(pipeline == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to generate pipeline.");
		return NULL;
	}

	if(release_shader)
	{
		SDL_ReleaseGPUShader(context.device, vs->shader);
		SDL_ReleaseGPUShader(context.device, fs->shader);
	}

	return pipeline;
}

bool Graphics_CreatePipelineSkybox(const char *path_vs,
										const char *path_fs)
{
	Shader modelvsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&modelvsshader, path_vs, "main", SHADERSTAGE_VERTEX, 0, 1, 0, 0))
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return false;
	}
	Shader modelfsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&modelfsshader, path_fs, "main", SHADERSTAGE_FRAGMENT, 1, 0, 0, 0))
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
		.vertex_shader = modelvsshader.shader,
		.fragment_shader = modelfsshader.shader
	};
	pipelines.skybox = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	SDL_ReleaseGPUShader(context.device, modelvsshader.shader);
	SDL_ReleaseGPUShader(context.device, modelfsshader.shader);

	return true;
}
