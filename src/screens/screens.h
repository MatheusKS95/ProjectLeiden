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

typedef enum CurrentScreen
{
	SCREEN_SPLASH = 0,
	//the following are TEMPORARY
	SCREEN_TEST,
	SCREEN_TEST2
} CurrentScreen;

typedef struct LeidenContext
{
	SDL_Window *window;
	SDL_GPUDevice *device;
} LeidenContext;

typedef struct EffectBuffers
{
	SDL_GPUBuffer *vbuffer;
	SDL_GPUBuffer *ibuffer;
} EffectBuffers;

typedef struct EffectVertex
{
	float x, y, z;
	float u, v;
} EffectVertex;

extern CurrentScreen current_screen;
extern LeidenContext drawing_context;
extern bool exit_signal;

/* HELPERS */
//BEGIN HELPERS

void SCR_CreateEffectBuffers(EffectBuffers *buffers);
void SCR_ReleaseEffectBuffers(EffectBuffers *buffers);
SDL_GPUGraphicsPipeline *SCR_CreateSimplePipeline(SDL_GPUShader *vs,
													SDL_GPUShader *fs,
													bool release_shaders);

//END HELPERS

/* SCREEN CONTROLS
 * Manages screens
*/
//BEGIN SCREEN CONTROLS

void SCR_SetContext(SDL_Window *window, SDL_GPUDevice *device);

bool SCR_Setup();

void SCR_Input(SDL_Event event);

void SCR_Iterate();

void SCR_Draw();

void SCR_Destroy();

//END SCREEN CONTROLS

/* SPLASH SCREEN
 * First screen on startup
*/
//BEGIN SPLASH

bool SplashScreen_Setup();

void SplashScreen_Input(SDL_Event event);

void SplashScreen_Iterate();

void SplashScreen_Draw();

void SplashScreen_Destroy();

//END SPLASH

/* TEST SCREEN 1
 * Temporary test for retro style
*/
//BEGIN TEST 1

bool TestScreen1_Setup();

void TestScreen1_Input(SDL_Event event);

void TestScreen1_Iterate();

void TestScreen1_Draw();

void TestScreen1_Destroy();

//END TEST 1

/* TEST SCREEN 2
 * Temporary test for collision detection
*/
//BEGIN TEST 2

bool TestScreen2_Setup();

void TestScreen2_Input(SDL_Event event);

void TestScreen2_Iterate();

void TestScreen2_Draw();

void TestScreen2_Destroy();

//END TEST 2
