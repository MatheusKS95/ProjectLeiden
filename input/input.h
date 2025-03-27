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
