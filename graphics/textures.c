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
 * @file textures.c
 * @brief Implementation file for texture stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <fileio.h>
#include <graphics.h>

Sampler* Graphics_GenerateSampler(SamplerFilter filter, SamplerMode mode)
{
	SDL_GPUFilter sdlfilter;
	SDL_GPUSamplerMipmapMode sdlmipmapmode;
	SDL_GPUSamplerAddressMode sdlsam;
	switch(filter)
	{
		case SAMPLER_FILTER_LINEAR:
		{
			sdlfilter = SDL_GPU_FILTER_LINEAR;
			sdlmipmapmode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
			break;
		}
		case SAMPLER_FILTER_NEAREST:
		{
			sdlfilter = SDL_GPU_FILTER_NEAREST;
			sdlmipmapmode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
			break;
		}
		default:
		{
			sdlfilter = SDL_GPU_FILTER_LINEAR;
			sdlmipmapmode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
			break;
		}
	}

	switch(mode)
	{
		case SAMPLER_MODE_REPEAT: sdlsam = SDL_GPU_SAMPLERADDRESSMODE_REPEAT; break;
		case SAMPLER_MODE_CLAMPTOEDGE: sdlsam = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE; break;
		case SAMPLER_MODE_MIRROREDREPEAT: sdlsam = SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT; break;
		default: sdlsam = SDL_GPU_SAMPLERADDRESSMODE_REPEAT; break;
	}

	SDL_GPUSamplerCreateInfo samplercreateinfo = { 0 };
	samplercreateinfo.min_filter = sdlfilter;
	samplercreateinfo.mag_filter = sdlfilter;
	samplercreateinfo.mipmap_mode = sdlmipmapmode;
	samplercreateinfo.address_mode_u = sdlsam;
	samplercreateinfo.address_mode_v = sdlsam;
	samplercreateinfo.address_mode_w = sdlsam;

	Sampler *result = SDL_CreateGPUSampler(context.device, &samplercreateinfo);
	return result;
}

void Graphics_ReleaseSampler(Sampler *sampler)
{
	SDL_ReleaseGPUSampler(context.device, sampler);
}

bool Graphics_LoadTextureFromMem(Texture2D *texture,
									uint8_t *buffer, size_t size)
{
	if(texture == NULL)
	{
		return false;
	}
	SDL_PixelFormat format;
	SDL_IOStream *stream;
	stream = SDL_IOFromMem(buffer, size);
	texture->surface = IMG_Load_IO(stream, true);
	if(texture->surface == NULL)
	{
		return false;
	}
	format = SDL_PIXELFORMAT_ABGR8888;
	if(texture->surface->format != format)
	{
		SDL_Surface *next = SDL_ConvertSurface(texture->surface, format);
		SDL_DestroySurface(texture->surface);
		texture->surface = next;
	}

	SDL_GPUTextureCreateInfo texcreateinfo = { 0 };
	texcreateinfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	texcreateinfo.type = SDL_GPU_TEXTURETYPE_2D;
	texcreateinfo.width = texture->surface->w;
	texcreateinfo.height = texture->surface->h;
	texcreateinfo.layer_count_or_depth = 1;
	texcreateinfo.num_levels = 1;
	texcreateinfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	texture->texture = SDL_CreateGPUTexture(context.device, &texcreateinfo);

	if(texture->texture == NULL)
	{
		//TODO check for errors SDL_GetError
		return false;
	}
	return true;
}

bool Graphics_LoadTextureFromFS(Texture2D *texture,
								const char *path)
{
	size_t filesize;
	uint8_t *file = FileIOReadBytes(path, &filesize);
	return Graphics_LoadTextureFromMem(texture, file, filesize);
}

void Graphics_ReleaseTexture(Texture2D *texture)
{
	SDL_ReleaseGPUTexture(context.device, texture->texture);
	SDL_DestroySurface(texture->surface);
}

void Graphics_UploadTexture(Texture2D *texture)
{
	SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = texture->surface->w * texture->surface->h * 4
		}
	);

	Uint8* textureTransferPtr = SDL_MapGPUTransferBuffer(
		context.device,
		textureTransferBuffer,
		false
	);
	SDL_memcpy(textureTransferPtr, texture->surface->pixels, texture->surface->w * texture->surface->h * 4);
	SDL_UnmapGPUTransferBuffer(context.device, textureTransferBuffer);

	// Upload the transfer data to the GPU resources
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context.device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	SDL_UploadToGPUTexture(
		copyPass,
		&(SDL_GPUTextureTransferInfo) {
			.transfer_buffer = textureTransferBuffer,
			.offset = 0, /* Zeros out the rest */
		},
		&(SDL_GPUTextureRegion){
			.texture = texture->texture,
			.w = texture->surface->w,
			.h = texture->surface->h,
			.d = 1
		},
		false
	);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_ReleaseGPUTransferBuffer(context.device, textureTransferBuffer);
	return;
}

GPUTexture *Graphics_GenerateDepthTexture(int width, int height)
{
	return SDL_CreateGPUTexture(
		context.device,
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
}

GPUTexture *Graphics_GenerateRenderTexture(int width, int height)
{
	return SDL_CreateGPUTexture(
		context.device,
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
}

bool Graphics_SetupDefaultTextures(const char *path_d,
									const char *path_n,
									const char *path_s,
									const char *path_e)
{
	if(path_d == NULL || path_n == NULL || path_s == NULL || path_e == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: failed to load default textures, invalid paths.");
		return false;
	}

	if(SDL_strcmp(path_d, "") == 0 || SDL_strcmp(path_n, "") == 0 || SDL_strcmp(path_s, "") == 0 || SDL_strcmp(path_e, "") == 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: failed to load default textures, invalid paths.");
		return false;
	}

	if(!Graphics_LoadTextureFromFS(&default_textures.default_diffuse, path_d))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: failed to load default diffuse texture.");
		return false;
	}
	if(!Graphics_LoadTextureFromFS(&default_textures.default_normal, path_n))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: failed to load default normal texture.");
		return false;
	}
	if(!Graphics_LoadTextureFromFS(&default_textures.default_spec, path_s))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: failed to load default specular texture.");
		return false;
	}
	if(!Graphics_LoadTextureFromFS(&default_textures.default_emission, path_e))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: failed to load default emission texture.");
		return false;
	}

	Graphics_UploadTexture(&default_textures.default_diffuse);
	Graphics_UploadTexture(&default_textures.default_normal);
	Graphics_UploadTexture(&default_textures.default_spec);
	Graphics_UploadTexture(&default_textures.default_emission);

	return true;
}

void Graphics_ReleaseDefaultTextures()
{
	Graphics_ReleaseTexture(&default_textures.default_diffuse);
	Graphics_ReleaseTexture(&default_textures.default_normal);
	Graphics_ReleaseTexture(&default_textures.default_spec);
	Graphics_ReleaseTexture(&default_textures.default_emission);
}
