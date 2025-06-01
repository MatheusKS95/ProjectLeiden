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
#include <leiden.h>

CurrentScreen current_screen;

bool SCR_Setup()
{
	current_screen = SCREEN_SPLASH;
	SplashScreen_Setup();
	return false;
}

void SCR_Logic(InputState *state)
{
	switch(current_screen)
	{
		case SCREEN_SPLASH: SplashScreen_Logic(state); break;
		case SCREEN_DEMOSIMPLE: Simple_Logic(state); break;
		default: break;
	}
	return;
}

void SCR_Draw()
{
	switch(current_screen)
	{
		case SCREEN_SPLASH: SplashScreen_Draw(); break;
		case SCREEN_DEMOSIMPLE: Simple_Draw(); break;
		default: break;
	}
	return;
}

void SCR_Destroy()
{
	switch(current_screen)
	{
		case SCREEN_SPLASH: SplashScreen_Destroy(); break;
		case SCREEN_DEMOSIMPLE: Simple_Destroy(); break;
		default: break;
	}
	return;
}
