#include <iostream>

#include "utility/Log.h"
#include "GameEngine.h"


int main(int argc, char* argv[])
{
	GameEngine engine;

	while (!engine.IsExit())
	{
		engine.Update();
	}

	return 0;
}