#pragma once

#include <memory>
#include <random>

#include "Audio.h"
#include "Input.h"
#include "ResourceManager.h"

class GameModule
{
public:
	static std::shared_ptr<Input> input;
	static std::shared_ptr<ResourceManager> resources;
	static std::shared_ptr<Audio> audio;
	static std::mt19937 random_gen;

	static void Init();
};