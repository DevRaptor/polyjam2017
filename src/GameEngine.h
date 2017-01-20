#pragma once

#include <memory>

#include <SDL.h>
#include <glm/glm.hpp>

#include "GameState.h"
#include "rendering/Renderer.h"



class GameEngine
{
	using Clock = std::chrono::high_resolution_clock;

public:
	GameEngine();
	~GameEngine();

	void Update();

	bool IsExit() { return exit; }



private:
	bool exit = false;

	std::shared_ptr<GameState> state;

	int resolution_x = 800;
	int resolution_y = 600;

	std::shared_ptr<Renderer> renderer;

	std::chrono::microseconds lag;

	std::chrono::high_resolution_clock::time_point time_start;

	void HandleEvents();

};
