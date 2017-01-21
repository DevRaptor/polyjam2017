#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera
{
public:
	Camera(float fov, float near, float far);
	~Camera();

	glm::mat4 GetMVP();
	void Update();
	void Translate(glm::vec3 position);
	void LookAt(glm::vec3 position);

private:
	glm::mat4 projection;

	glm::vec3 position;
	glm::vec3 observedObjectsPosition;
	glm::quat rotation;

	float move_speed;
	float rotation_speed;

	glm::mat4 view;


	glm::vec3 GetUp();
	glm::vec3 GetRight();

	void Move(const glm::vec3& direction);
	void Rotate(const glm::quat &rot);
};
