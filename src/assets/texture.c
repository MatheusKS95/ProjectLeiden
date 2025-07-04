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
#include <SDL3_image/SDL_image.h>
#include <assets.h>
#include <fileio.h>

bool LoadTextureFile(SDL_GPUDevice *device, Texture2D *texture,
						const char *path)
{
	if(texture == NULL)
	{
		//TODO error message
		return false;
	}
	size_t filesize;
	Uint8 *buffer = FileIOReadBytes(path, &filesize);
	SDL_IOStream *stream;
	stream = SDL_IOFromMem(buffer, filesize);
	texture->surface = IMG_Load_IO(stream, true);
	if(texture->surface == NULL)
	{
		return false;
	}
	SDL_PixelFormat format = SDL_PIXELFORMAT_ABGR8888;
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
	texture->texture = SDL_CreateGPUTexture(device, &texcreateinfo);

	if(texture == NULL)
	{
		//TODO check for errors SDL_GetError
		return false;
	}

	SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
		device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = texture->surface->w * texture->surface->h * 4
		}
	);

	Uint8* textureTransferPtr = SDL_MapGPUTransferBuffer(
		device,
		textureTransferBuffer,
		false
	);
	SDL_memcpy(textureTransferPtr, texture->surface->pixels, texture->surface->w * texture->surface->h * 4);
	SDL_UnmapGPUTransferBuffer(device, textureTransferBuffer);

	// Upload the transfer data to the GPU resources
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
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
	SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);

	return true;
}

void ReleaseTexture2D(SDL_GPUDevice *device, Texture2D *texture)
{
	if(device == NULL || texture == NULL)
	{
		return;
	}
	SDL_ReleaseGPUTexture(device, texture->texture);
	SDL_DestroySurface(texture->surface);
}