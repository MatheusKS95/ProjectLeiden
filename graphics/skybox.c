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
 * @brief Implementation file for skybox stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <fileio.h>
#include <graphics.h>

//reference
static bool _loadtextureskybox_mem(Texture2D *texture,
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
	texcreateinfo.type = SDL_GPU_TEXTURETYPE_CUBE;
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

bool Graphics_LoadSkyboxFS(Skybox *skybox, const char *path_up,
							const char *path_down,
							const char *path_left,
							const char *path_right,
							const char *path_front,
							const char *path_back)
{
	if(skybox == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: failed to load skybox, invalid structure.");
		return false;
	}
	if(path_up == NULL || path_down == NULL || path_left == NULL || path_right == NULL || path_front == NULL || path_back == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: failed to load skybox, one file might be invalid.");
		return false;
	}
	//TODO check if empty

	skybox->vertex_buffer = SDL_CreateGPUBuffer(
				context.device,
				&(SDL_GPUBufferCreateInfo)
				{
					.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
					.size = sizeof(Vector3) * 24
				}
	);
	skybox->index_buffer = SDL_CreateGPUBuffer(
				context.device,
				&(SDL_GPUBufferCreateInfo)
				{
					.usage = SDL_GPU_BUFFERUSAGE_INDEX,
					.size = sizeof(Uint32) * 36
				}
	);

	const char* imagenames[] = {
		path_right, path_left, path_up,
		path_down, path_front, path_back
			//path_right, path_front, path_left,
			//path_up, path_down, path_back,
	};

	for(int i = 0; i < SDL_arraysize(imagenames); i++)
	{
		size_t filesize;
		Uint8 *buffer = FileIOReadBytes(imagenames[i], &filesize);
		SDL_PixelFormat format;
		SDL_IOStream *stream;
		stream = SDL_IOFromMem(buffer, filesize);
		skybox->surface[i] = IMG_Load_IO(stream, true);

		if(skybox->surface[i] == NULL)
		{
			//FIXME, gracefully exit, valgrind is going to hate me
			return false;
		}
		format = SDL_PIXELFORMAT_ABGR8888;
		if(skybox->surface[i]->format != format)
		{
			SDL_Surface *next = SDL_ConvertSurface(skybox->surface[i], format);
			SDL_DestroySurface(skybox->surface[i]);
			skybox->surface[i] = next;
		}
	}

	skybox->gputexture = SDL_CreateGPUTexture(
				context.device,
				&(SDL_GPUTextureCreateInfo)
				{
					.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
					.type = SDL_GPU_TEXTURETYPE_CUBE,
					.width = skybox->surface[0]->w,
					.height = skybox->surface[0]->h,
					.layer_count_or_depth = 6,
					.num_levels = 1,
					.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
				}
	);

	skybox->sampler = Graphics_GenerateSampler(SAMPLER_FILTER_LINEAR, SAMPLER_MODE_CLAMPTOEDGE);

	return true;
}

void Graphics_UploadSkybox(Skybox *skybox)
{
	//shape of skybox
	SDL_GPUTransferBuffer* buffer_transferbuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(Vector3) * 24) + (sizeof(Uint32) * 36)
		}
	);

	Vector3* buffer_transferdata = SDL_MapGPUTransferBuffer(
		context.device,
		buffer_transferbuffer,
		false
	);

	buffer_transferdata[0] = (Vector3) { -10, -10, -10 };
	buffer_transferdata[1] = (Vector3) { 10, -10, -10 };
	buffer_transferdata[2] = (Vector3) { 10, 10, -10 };
	buffer_transferdata[3] = (Vector3) { -10, 10, -10 };

	buffer_transferdata[4] = (Vector3) { -10, -10, 10 };
	buffer_transferdata[5] = (Vector3) { 10, -10, 10 };
	buffer_transferdata[6] = (Vector3) { 10, 10, 10 };
	buffer_transferdata[7] = (Vector3) { -10, 10, 10 };

	buffer_transferdata[8] = (Vector3) { -10, -10, -10 };
	buffer_transferdata[9] = (Vector3) { -10, 10, -10 };
	buffer_transferdata[10] = (Vector3) { -10, 10, 10 };
	buffer_transferdata[11] = (Vector3) { -10, -10, 10 };

	buffer_transferdata[12] = (Vector3) { 10, -10, -10 };
	buffer_transferdata[13] = (Vector3) { 10, 10, -10 };
	buffer_transferdata[14] = (Vector3) { 10, 10, 10 };
	buffer_transferdata[15] = (Vector3) { 10, -10, 10 };

	buffer_transferdata[16] = (Vector3) { -10, -10, -10 };
	buffer_transferdata[17] = (Vector3) { -10, -10, 10 };
	buffer_transferdata[18] = (Vector3) { 10, -10, 10 };
	buffer_transferdata[19] = (Vector3) { 10, -10, -10 };

	buffer_transferdata[20] = (Vector3) { -10, 10, -10 };
	buffer_transferdata[21] = (Vector3) { -10, 10, 10 };
	buffer_transferdata[22] = (Vector3) { 10, 10, 10 };
	buffer_transferdata[23] = (Vector3) { 10, 10, -10 };

	//TODO maybe separate this
	Uint32* indexdata = (Uint32*) &buffer_transferdata[24];
	Uint32 indices[] = {
		 0,  1,  2,  0,  2,  3,
		 6,  5,  4,  7,  6,  4,
		 8,  9, 10,  8, 10, 11,
		14, 13, 12, 15, 14, 12,
		16, 17, 18, 16, 18, 19,
		22, 21, 20, 23, 22, 20
	};
	SDL_memcpy(indexdata, indices, sizeof(indices));
	SDL_UnmapGPUTransferBuffer(context.device, buffer_transferbuffer);

	//skybox textures
	const Uint32 imagesize = skybox->surface[0]->w * skybox->surface[0]->h * 4;
	const Uint32 side_size = skybox->surface[0]->w;
	SDL_GPUTransferBuffer* texture_transferbuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = imagesize * 6
		}
	);
	Uint8* texture_transferdata = SDL_MapGPUTransferBuffer(
		context.device,
		texture_transferbuffer,
		false
	);

	for(int i = 0; i < 6; i++)
	{
		SDL_memcpy(texture_transferdata + (imagesize * i), skybox->surface[i]->pixels, imagesize);
		SDL_DestroySurface(skybox->surface[i]);
	}
	SDL_UnmapGPUTransferBuffer(context.device, texture_transferbuffer);

	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

	//TODO do the upload here
	SDL_UploadToGPUBuffer(
		copypass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = buffer_transferbuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = skybox->vertex_buffer,
			.offset = 0,
			.size = sizeof(Vector3) * 24
		},
		false
	);

	SDL_UploadToGPUBuffer(
		copypass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = buffer_transferbuffer,
			.offset = sizeof(Vector3) * 24
		},
		&(SDL_GPUBufferRegion) {
			.buffer = skybox->index_buffer,
			.offset = 0,
			.size = sizeof(Uint32) * 36
		},
		false
	);

	for (int i = 0; i < 6; i += 1) {
		SDL_UploadToGPUTexture(
			copypass,
			&(SDL_GPUTextureTransferInfo) {
				.transfer_buffer = texture_transferbuffer,
				.offset = imagesize * i
			},
			&(SDL_GPUTextureRegion) {
				.texture = skybox->gputexture,
				.layer = i,
				.w = side_size,
				.h = side_size,
				.d = 1,
			},
			false
		);
	}

	SDL_EndGPUCopyPass(copypass);
	SDL_ReleaseGPUTransferBuffer(context.device, buffer_transferbuffer);
	SDL_ReleaseGPUTransferBuffer(context.device, texture_transferbuffer);

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}