#pragma once

#include <glm/glm.hpp>

#include <SDL.h>

class Input
{
public:
	Input();
	~Input();

	void Update();

	bool GetKeyState(SDL_Scancode key_code);

	glm::vec2 GetMousePos();
	bool IsLeftMouseButtonPressed() { return left_mouse_button_pressed; }

private:
	Uint8* key_state;
	bool left_mouse_button_pressed;
};
