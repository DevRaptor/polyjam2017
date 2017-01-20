#pragma once

#include <string>
#include <vector>

#define GLEW_STATIC //needed to static link GLEW
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Mesh
{
public:
	Mesh(const std::string& model_name, glm::vec3 pos = glm::vec3(0, 0, 0));
	~Mesh();

	void Draw();
	void SetPosition(glm::vec3 pos);

	glm::mat4 GetTransform();

private:
	int vertex_amount;

	GLuint vao;
	GLuint vbo_vertex;
	GLuint vbo_uv;
	GLuint vbo_normal;

	GLuint texture;

	glm::vec3 position;

	bool LoadOBJ(const std::string& file_name,
		std::vector<glm::vec3>& out_vertices,
		std::vector<glm::vec2>& out_uvs,
		std::vector<glm::vec3>& out_normals);
};
