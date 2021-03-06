#include "Camera.h"

#include <iostream>

#include "modules/GameModule.h"

Camera::Camera(float fov, float near, float far)
	: view(1.0f), shakeStartTime{ std::chrono::high_resolution_clock::now() }, isShaking{ false }
{
	move_speed = GameModule::resources->GetFloatParameter("camera_move_speed");
	rotation_speed = GameModule::resources->GetFloatParameter("camera_rotation_speed");
	float resolution_x = GameModule::resources->GetIntParameter("resolution_x");
	float resolution_y = GameModule::resources->GetIntParameter("resolution_y");
	float ratio = resolution_x / resolution_y;

	position = glm::vec3(1, 1, 0);
	observedObjectsPosition = glm::vec3(0, 0, 0);

	projection = glm::perspective(glm::radians(fov), ratio, near, far);
}

Camera::~Camera()
{
}

glm::mat4 Camera::GetMVP()
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), -position);
	//glm::mat4 view = glm::mat4_cast(glm::conjugate(rotation));
	//glm::mat4 view = glm::lookAt(position, observedObjectsPosition, glm::vec3(-1, 0, 0));
	return projection * view * model;
}

void Camera::Update()
{
	//glm::vec3 pos = glm::vec3(0, 0, 0);

	/*if (GameModule::input->GetKeyState(SDL_SCANCODE_A))
		pos.x = -1.0f;
	else if (GameModule::input->GetKeyState(SDL_SCANCODE_D))
		pos.x = 1.0f;

	if (GameModule::input->GetKeyState(SDL_SCANCODE_S))
		pos.z = 1.0f;
	else if (GameModule::input->GetKeyState(SDL_SCANCODE_W))
		pos.z = -1.0f;

	if (glm::length(pos) > 0.0f)
	{
		pos = glm::normalize(pos) * move_speed;
		Move(pos);
	}

	if (GameModule::input->IsLeftMouseButtonPressed())
	{
		glm::vec2 mouse_delta = GameModule::input->GetMouseDeltaPos();
		glm::vec2 v = mouse_delta * rotation_speed;
		glm::quat rot = glm::angleAxis(-v.x, glm::vec3(0, 1, 0)) * glm::angleAxis(-v.y, GetRight());
		Rotate(rot);
	}*/
}


glm::vec3 Camera::GetUp() 
{
	return glm::vec3(rotation * glm::vec4(0, 1, 0, 1));
}

glm::vec3 Camera::GetRight() 
{
	return glm::vec3(rotation * glm::vec4(1, 0, 0, 1));
}


void Camera::Move(const glm::vec3& direction)
{
	position += glm::vec3(rotation * glm::vec4(direction, 1));
}

void Camera::Rotate(const glm::quat &rot) 
{
	rotation = glm::normalize(rot) * rotation;
}

void Camera::Translate(glm::vec3 position)
{
	this->position = position;
}

void Camera::LookAt(glm::vec3 position)
{
	observedObjectsPosition = position;
	view = glm::lookAt(this->position, observedObjectsPosition, glm::vec3(-1, 0, 0));
}

void Camera::Shake()
{
	if (isShaking && (shakeStartTime + std::chrono::milliseconds(GameModule::resources->GetIntParameter("shake_duration")) > std::chrono::high_resolution_clock::now()))
	{
		std::uniform_real_distribution<double> random(0, 1.0);
		position = position + glm::vec3(random(GameModule::random_gen), random(GameModule::random_gen), random(GameModule::random_gen));
	}
}

void Camera::StartShaking()
{
	isShaking = true;
	shakeStartTime = std::chrono::high_resolution_clock::now();
}