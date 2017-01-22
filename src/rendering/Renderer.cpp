#include "Renderer.h"

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "modules/GameModule.h"
#include "utility/Log.h"

Renderer::Renderer(int resolution_x, int resolution_y)
{
	window = SDL_CreateWindow(GameModule::resources->GetStringParameter("game_title").c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		resolution_x, resolution_y, SDL_WINDOW_OPENGL);

	if (window == NULL)
	{
		Logger::Log("Could not create window: ", SDL_GetError(), "\n");
		std::exit(EXIT_FAILURE);
	}


	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	context = SDL_GL_CreateContext(window);

	SDL_GL_MakeCurrent(window, context);
	SDL_GL_SetSwapInterval(GameModule::resources->GetIntParameter("vsync"));

	int gl_major, gl_minor;
	glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
	glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
	Logger::Log("Video driver: ", SDL_GetCurrentVideoDriver(), "\n");
	Logger::Log("GL context version: ", gl_major, ".", gl_minor, "\n");

	//Init GLEW
	glewExperimental = true;
	glewInit();

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0, 0, 0, 0);
	glViewport(0, 0, resolution_x, resolution_y);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);

	shader_program = std::make_shared<ShaderProgram>("data/shaders/color.vert", "data/shaders/color.frag");

	shader_program->UseProgram();

	mvp_uniform = glGetUniformLocation(shader_program->GetProgram(), "mvp");
	transform_uniform = glGetUniformLocation(shader_program->GetProgram(), "transform");
	fadeout_uniform = glGetUniformLocation(shader_program->GetProgram(), "fadeout");


	shader_gui = std::make_shared<ShaderProgram>("data/shaders/gui.vert", "data/shaders/gui.frag");

	shader_gui->UseProgram();

	mvp_gui_uniform = glGetUniformLocation(shader_gui->GetProgram(), "mvp");
	transform_gui_uniform = glGetUniformLocation(shader_gui->GetProgram(), "transform");
	
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(0.01, 50, 0), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	gui_mvp = proj * View * Model;


}

Renderer::~Renderer()
{
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
}

void Renderer::Render(std::shared_ptr<GameState> game_state)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	shader_program->UseProgram();

	mvp = game_state->camera.GetMVP();
	glUniformMatrix4fv(mvp_uniform, 1, GL_FALSE, &mvp[0][0]);

	fadeout = game_state->get_fadeout();
	glUniform1f(fadeout_uniform, fadeout);

	if (game_state->floor_mesh)
	{
		//glm::mat4 mat(1.0f);
		glUniformMatrix4fv(transform_uniform, 1, GL_FALSE, glm::value_ptr(game_state->floor_transform));
		game_state->floor_mesh->Draw();
	}

	for (auto ptr : game_state->players)
	{
		glUniformMatrix4fv(transform_uniform, 1, GL_FALSE, glm::value_ptr(ptr->transform_mat));
		ptr->Draw();
	}

	for (auto ptr : game_state->entities)
	{
		glUniformMatrix4fv(transform_uniform, 1, GL_FALSE, glm::value_ptr(ptr->transform_mat));
		ptr->Draw();
	}


	shader_gui->UseProgram();

	glUniformMatrix4fv(mvp_gui_uniform, 1, GL_FALSE, glm::value_ptr(gui_mvp));

	for (auto ptr : game_state->gui)
	{
		glUniformMatrix4fv(transform_gui_uniform, 1, GL_FALSE, glm::value_ptr(ptr->GetTransform()));
		ptr->Draw();
	}

	if (game_state->next_player)
	{
		glUniformMatrix4fv(transform_gui_uniform, 1, GL_FALSE, glm::value_ptr(game_state->next_player->GetTransform()));
		game_state->next_player->Draw();
	}

	if (game_state->portrait)
	{
		glUniformMatrix4fv(transform_gui_uniform, 1, GL_FALSE, glm::value_ptr(game_state->portrait->GetTransform()));
		game_state->portrait->Draw();
	}

	SDL_GL_SwapWindow(window);
}
