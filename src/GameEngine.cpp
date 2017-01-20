#include "GameEngine.h"

#include <iomanip>
#include <chrono>

#include "GameModule.h"
#include "utility/Log.h"

using namespace std::chrono_literals;

//fixed time step, 1 / 60 [s] = 16 ms
constexpr std::chrono::milliseconds time_step(16ms);

GameEngine::GameEngine()
	: lag(0), time_start(Clock::now())
{
	Logger::InitLog();
	Logger::Log("===================================\n");
	Logger::Log("============Space Ships============\n");
	Logger::Log("===================================\n");

	std::time_t actual_date = std::time(nullptr);;
	Logger::Log("Start date: ", std::put_time(std::localtime(&actual_date), "%c \n"));

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		Logger::Log("Init SDL\n");
	}
	else
	{
		Logger::Log("Cannot init SDL!\n");
		std::exit(EXIT_FAILURE);
	}

	GameModule::Init();

	Logger::Log("Platform: ", SDL_GetPlatform(), "\n");

	renderer = std::make_shared<Renderer>(GameModule::resources->GetIntParameter("resolution_x"),
		GameModule::resources->GetIntParameter("resolution_y"));



	state = std::make_shared<GameState>();

}

GameEngine::~GameEngine()
{
	state.reset();
	renderer.reset(); //close context and window before SDL_Quit()
	SDL_Quit();

	Logger::Log("===================================\n");
	Logger::Log("Close game\n");

	Logger::CloseLog();
}

void GameEngine::Update()
{
	HandleEvents();

	GameModule::input->Update();
	if (GameModule::input->GetKeyState(SDL_SCANCODE_ESCAPE))
		exit = true;

	auto delta_time = Clock::now() - time_start;
	time_start = Clock::now();

	lag += std::chrono::duration_cast<std::chrono::microseconds>(delta_time);

	//fixed time step update
	while (lag >= time_step)
	{
		lag -= time_step;

		state->Update(time_step);
	}

	renderer->Render(state);
}

void GameEngine::HandleEvents()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			exit = true;
	}
}

