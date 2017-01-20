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

	int x, y;
	SDL_GetMouseState(&x, &y);
	mouse_pos = glm::vec2(x, y);

	if (!left_mouse_button_pressed)
		last_mouse_pos = mouse_pos;

}

bool Input::GetKeyState(SDL_Scancode key_code)
{
	if (key_state[key_code] != 0)
		return true;

	return false;
}

glm::vec2 Input::GetMousePos()
{
	return mouse_pos;
}

glm::vec2 Input::GetMouseDeltaPos()
{
	glm::vec2 delta = mouse_pos - last_mouse_pos;
	last_mouse_pos = mouse_pos;
	
	return delta;
}
