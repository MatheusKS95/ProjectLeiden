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

#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>

typedef struct InputState
{
	bool keys[SDL_SCANCODE_COUNT];
	bool mouse_buttons[3]; // Left, middle and right, idk how to deal with multiple mouse buttons
	int mouse_x, mouse_y; // Mouse position
} InputState;

void Input_GetState(InputState *state, SDL_Event *event);

//bool Input_IsKeyPressed() //need to create my own scancodes/keycodes

bool Input_IsKeyPressedTEMP(InputState *state, SDL_Scancode scancode);

#endif
