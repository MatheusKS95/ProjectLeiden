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

#include <input.h>

void Input_GetState(InputState *state, SDL_Event *event)
{
	if(event->type == SDL_EVENT_KEY_DOWN)
	{
		state->keys[event->key.scancode] = true;
	}
	if(event->type == SDL_EVENT_KEY_UP)
	{
		state->keys[event->key.scancode] = false;
	}
	if(event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
	{
		state->mouse_buttons[event->button.button] = true;
	}
	if(event->type == SDL_EVENT_MOUSE_BUTTON_UP)
	{
		state->mouse_buttons[event->button.button] = false;
	}
	if(event->type == SDL_EVENT_MOUSE_MOTION)
	{
		state->mouse_x = event->motion.xrel;
		state->mouse_y = event->motion.yrel;
	}
	return;
}

bool Input_IsKeyPressedTEMP(InputState *state, SDL_Scancode scancode)
{
	return state->keys[scancode];
}
