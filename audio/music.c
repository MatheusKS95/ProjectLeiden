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
 * @file music.c
 * @brief Implementation file for music stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <SDL3/SDL.h>
#include "miniaudio.h"
#include <audio.h>
#include <fileio.h>

bool Audio_LoadMusicFromMem(Music *music, uint8_t *data, size_t size, bool looping)
{
	if(music == NULL || data == NULL) return false;

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Audio: Loading music...");

	music->sound = SDL_malloc(sizeof(*music->sound));
	music->decoder = SDL_malloc(sizeof(*music->decoder));
	music->source = (uint8_t*)SDL_malloc(sizeof(uint8_t) * size);
	SDL_memcpy(music->source, data, size);
	music->size = size;

	ma_result decode_result = ma_decoder_init_memory(music->source, music->size, NULL, music->decoder);
	if(decode_result != MA_SUCCESS)
	{
		return false;
	}
	ma_result sound_result = ma_sound_init_from_data_source(audiocontext.music_engine, music->decoder, 0, NULL, music->sound);
	if(sound_result != MA_SUCCESS)
	{
		return false;
	}

	ma_sound_set_looping(music->sound, looping);

	return true;
}

bool Audio_LoadMusicFromFS(Music *music, const char *path, bool looping)
{
	if(music == NULL)
	{
		return false;
	}

	size_t filesize;
	uint8_t *musicfile = FileIOReadBytes(path, &filesize);
	if(musicfile == NULL)
	{
		SDL_free(music);
		return NULL;
	}
	if(!Audio_LoadMusicFromMem(music, musicfile, filesize, looping))
	{
		SDL_free(music);
		SDL_free(musicfile);
		return NULL;
	}

	SDL_free(musicfile);
	return music;
}

void Audio_FreeMusic(Music *music)
{
	if(music == NULL) return;
	ma_sound_uninit(music->sound);
	ma_decoder_uninit(music->decoder);
	SDL_free(music->decoder);
	SDL_free(music->sound);
	SDL_free(music->source);
	return;
}

void Audio_PlayMusic(Music *music)
{
	if(music == NULL) return;
	ma_sound_start(music->sound);
	return;
}

void Audio_PauseMusic(Music *music)
{
	//FIXME, not streaming
	if(music == NULL) return;
	ma_sound_stop(music->sound);
	return;
}

void Audio_StopMusic(Music *music)
{
	if(music == NULL) return;
	ma_sound_stop(music->sound);
	return;
}
