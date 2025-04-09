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

bool Audio_LoadSoundFromMem(Sound *sound, uint8_t *data, size_t size)
{
	if(sound == NULL || data == NULL) return false;

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Audio: Loading sound...");

	sound->sound = SDL_malloc(sizeof(*sound->sound));
	sound->decoder = SDL_malloc(sizeof(*sound->decoder));
	sound->source = (uint8_t*)SDL_malloc(sizeof(uint8_t) * size);
	SDL_memcpy(sound->source, data, size);
	sound->size = size;

	ma_result decode_result = ma_decoder_init_memory(sound->source, sound->size, NULL, sound->decoder);
	if(decode_result != MA_SUCCESS)
	{
		return false;
	}
	ma_result sound_result = ma_sound_init_from_data_source(audiocontext.sound_engine, sound->decoder, 0, NULL, sound->sound);
	if(sound_result != MA_SUCCESS)
	{
		return false;
	}

	ma_sound_set_positioning(sound->sound, ma_positioning_absolute);
	ma_sound_set_position(sound->sound, sound->position.x, sound->position.y, sound->position.z);
	ma_sound_set_direction(sound->sound, sound->direction.x, sound->direction.y, sound->direction.z);
	ma_sound_set_velocity(sound->sound, sound->velocity.x, sound->velocity.y, sound->velocity.z);
	ma_sound_set_volume(sound->sound, sound->volume);
	//TODO cone;

	return true;
}

bool Audio_LoadSoundFromFS(Sound *sound, const char *path)
{
	if(sound == NULL)
	{
		return false;
	}

	//TODO create function to set particular sound volume (separate from overall volume for sound effects)
	sound->volume = 1.0f;
	size_t filesize;
	uint8_t *soundfile = FileIOReadBytes(path, &filesize);
	if(soundfile == NULL)
	{
		SDL_free(sound);
		return false;
	}
	if(!Audio_LoadSoundFromMem(sound, soundfile, filesize))
	{
		SDL_free(sound);
		SDL_free(soundfile);
		return false;
	}

	SDL_free(soundfile);
	soundfile = NULL;

	return true;
}

void Audio_FreeSound(Sound *sound)
{
	if(sound == NULL) return;
	ma_sound_uninit(sound->sound);
	ma_decoder_uninit(sound->decoder);
	SDL_free(sound->source);
	SDL_free(sound->sound);
	return;
}

void Audio_UpdatePositioningSound(Sound *sound, AudioVec3 position, AudioVec3 direction, AudioVec3 velocity, float inner_cone, float outer_cone)
{
	if(sound == NULL) return;
	sound->position = position;
	sound->direction = direction;
	sound->velocity = velocity;
	sound->inner_cone = inner_cone;
	sound->outer_cone = outer_cone;

	ma_sound_set_position(sound->sound, sound->position.x, sound->position.y, sound->position.z);
	ma_sound_set_direction(sound->sound, sound->direction.x, sound->direction.y, sound->direction.z);
	ma_sound_set_velocity(sound->sound, sound->velocity.x, sound->velocity.y, sound->velocity.z);
	//TODO cone;
	return;
}

void Audio_PlaySound(Sound *sound)
{
	if(sound == NULL) return;
	//ma_result result = ma_sound_start(sound->sound);
	ma_sound_start(sound->sound);
	return;
}
