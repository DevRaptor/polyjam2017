#pragma once

#define GLEW_STATIC //needed to static link GLEW
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

class Mesh
{
public:
	Mesh();
	~Mesh();

	void Draw();

private:
	int vertex_amount;

	GLuint vao;
	GLuint vbo_vertex;
	GLuint vbo_color;
};
