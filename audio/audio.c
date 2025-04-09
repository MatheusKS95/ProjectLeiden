/**
 * @file megaboomblaster.c
 * @brief megaboomblaster.h implementation file
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <audio.h>

AudioContext audiocontext;

bool Audio_Init()
{
	audiocontext.sound_engine = SDL_malloc(sizeof(*audiocontext.sound_engine));
	if(audiocontext.sound_engine == NULL) return false;
	ma_result result;
	result = ma_engine_init(NULL, audiocontext.sound_engine);
	if(result != MA_SUCCESS)
	{
		return false;
	}
	audiocontext.music_engine = SDL_malloc(sizeof(*audiocontext.music_engine));
	if(audiocontext.music_engine == NULL) return false;
	result = ma_engine_init(NULL, audiocontext.music_engine);
	if(result != MA_SUCCESS)
	{
		return false;
	}
	return true;
}

void Audio_Deinit()
{
	ma_engine_uninit(audiocontext.sound_engine);
	SDL_free(audiocontext.sound_engine);
	ma_engine_uninit(audiocontext.music_engine);
	SDL_free(audiocontext.music_engine);
	return;
}

void Audio_LogNameCurrentDevice()
{
	ma_device *device_music = ma_engine_get_device(audiocontext.music_engine);
	ma_device_info info_music;
	ma_result result = ma_device_get_info(device_music, ma_device_type_playback, &info_music);
	if(result == MA_SUCCESS)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Current audio device for music is %s", info_music.name);
	}
	ma_device *device_sound = ma_engine_get_device(audiocontext.sound_engine);
	ma_device_info info_sound;
	result = ma_device_get_info(device_music, ma_device_type_playback, &info_sound);
	if(result == MA_SUCCESS)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Current audio device for sounds is %s", info_sound.name);
	}
}

void Audio_SetVolumeMusic(float volume)
{
	//0 = silence; 1 = full volume; > 1 amplified
	ma_engine_set_volume(audiocontext.music_engine, volume);
}
