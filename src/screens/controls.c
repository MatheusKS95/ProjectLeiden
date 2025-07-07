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

#include <screens.h>

CurrentScreen current_screen;
RockportContext drawing_context;
bool exit_signal;

void SCR_SetContext(SDL_Window *window, SDL_GPUDevice *device)
{
	drawing_context.window = window;
	drawing_context.device = device;
}

bool SCR_Setup()
{
	current_screen = SCREEN_SPLASH;
	SplashScreen_Setup();
	exit_signal = false;
	return false;
}

void SCR_Logic(SDL_Event event)
{
	switch(current_screen)
	{
		case SCREEN_SPLASH: SplashScreen_Logic(event); break;
		case SCREEN_TEST: TestScreen1_Logic(event); break;
		case SCREEN_TEST2: TestScreen2_Logic(event); break;
		default: break;
	}
	return;
}

void SCR_Draw()
{
	switch(current_screen)
	{
		case SCREEN_SPLASH: SplashScreen_Draw(); break;
		case SCREEN_TEST: TestScreen1_Draw(); break;
		case SCREEN_TEST2: TestScreen2_Draw(); break;
		default: break;
	}
	return;
}

void SCR_Destroy()
{
	switch(current_screen)
	{
		case SCREEN_SPLASH: SplashScreen_Destroy(); break;
		case SCREEN_TEST: TestScreen1_Destroy(); break;
		case SCREEN_TEST2: TestScreen2_Destroy(); break;
		default: break;
	}
	return;
}
