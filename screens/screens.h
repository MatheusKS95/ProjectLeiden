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

#include <SDL3/SDL_stdinc.h>
#include <input.h>

/* SCREEN CONTROLS
 * Manages screens
*/
//BEGIN SCREEN CONTROLS

typedef enum CurrentScreen
{
	SCREEN_SPLASH = 0,
	SCREEN_DEMOSIMPLE
} CurrentScreen;

extern CurrentScreen current_screen;

bool SCR_Setup();

void SCR_Logic(InputState *state);

void SCR_Draw();

void SCR_Destroy();

//END SCREEN CONTROLS

/* SPLASH SCREEN
 * First screen on startup
*/
//BEGIN SPLASH

bool SplashScreen_Setup();

void SplashScreen_Logic(InputState *state);

void SplashScreen_Draw();

void SplashScreen_Destroy();

//END SPLASH

/* SIMPLE RENDERING SCREEN
 * Intended to be used for testing.
*/
//BEGIN SIMPLE

bool Simple_Setup();

void Simple_Logic(InputState *state);

void Simple_Draw();

void Simple_Destroy();

//END SIMPLE
