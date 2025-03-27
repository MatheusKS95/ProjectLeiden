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