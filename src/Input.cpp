#include "Input.h"

Input::Input()
{
	key_state = const_cast<Uint8*>(SDL_GetKeyboardState(NULL));
	left_mouse_button_pressed = false;
}

Input::~Input()
{
}

void Input::Update()
{
	key_state = const_cast<Uint8*>(SDL_GetKeyboardState(NULL));

	left_mouse_button_pressed = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LEFT;
}

bool Input::GetKeyState(SDL_Scancode key_code)
{
	if (key_state[key_code] != 0)
		return true;

	return false;
}

glm::vec2 Input::GetMousePos()
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	return glm::vec2(x, y);
}
