#include "GameModule.h"

#include <ctime>

std::shared_ptr<Input> GameModule::input;
std::shared_ptr<ResourceManager> GameModule::resources;
std::mt19937 GameModule::random_gen;

void GameModule::Init()
{
	input = std::make_shared<Input>();
	resources = std::make_shared<ResourceManager>();

	srand(time(NULL));

	random_gen.seed(rand());
}