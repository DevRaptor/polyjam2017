#pragma once

#include <memory>
#include <random>

#include "Input.h"
#include "ResourceManager.h"

class GameModule
{
public:
	static std::shared_ptr<Input> input;
	static std::shared_ptr<ResourceManager> resources;
	static std::mt19937 random_gen;

	static void Init();
};