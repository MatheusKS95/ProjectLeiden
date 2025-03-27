/**
 * @file megaboomblaster.h
 * @brief Main megaboomblaster's header
 *
 * This header have declarations for OpenAL-specific stuff.
 * Controls all audio and music, except audio file loading.
 * TODO: allow to select device
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <SDL3/SDL.h>
#include <miniaudio.h>

//TODO FIXME, separate implementations between music.c,
//sounds.c and megaboomblaster.c

//BASIC
typedef struct AudioContext
{
	ma_engine *sound_engine;
	ma_engine *music_engine;
} AudioContext;

extern AudioContext audiocontext;

bool Audio_Init();

void Audio_Deinit();

void Audio_LogNameCurrentDevice();

void Audio_SetVolumeMusic(float volume);

//MUSIC
typedef struct Music
{
	ma_sound *sound;
	ma_decoder *decoder;
	size_t size;
	uint8_t *source;
} Music;

bool Audio_LoadMusicFromMem(Music *music, uint8_t *data, size_t size, bool looping);

bool Audio_LoadMusicFromFS(Music *music, const char *path, bool looping);

void Audio_FreeMusic(Music *music);

void Audio_PlayMusic(Music *music);

void Audio_PauseMusic(Music *music);

void Audio_StopMusic(Music *music);

//SOUND
typedef struct AudioVec3
{
	float x;
	float y;
	float z;
} AudioVec3;

typedef struct Sound
{
	ma_sound *sound;
	ma_decoder *decoder;
	size_t size;
	uint8_t *source;
	AudioVec3 position;
	AudioVec3 direction;
	AudioVec3 velocity;
	float inner_cone;
	float outer_cone;
	float volume;
} Sound;

bool Audio_LoadSoundFromMem(Sound *sound, uint8_t *data, size_t size);

bool Audio_LoadSoundFromFS(Sound *sound, const char *path);

void Audio_FreeSound(Sound *sound);

void Audio_UpdatePositioningSound(Sound *sound, AudioVec3 position, AudioVec3 direction, AudioVec3 velocity, float inner_cone, float outer_cone);

void Audio_PlaySound(Sound *sound);

#endif
