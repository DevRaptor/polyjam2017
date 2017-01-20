#pragma once

#include <string>

#define GLEW_STATIC //needed to static link GLEW
#include <GL/glew.h>
#include <glm/glm.hpp>


class ShaderProgram
{
public:
	ShaderProgram(const std::string& vert_path, const std::string& frag_path);
	~ShaderProgram();

	void UseProgram();
	GLuint GetProgram();

private:
	GLuint program;

	//to avoid bind same program must check last binded program
	static GLuint active_program;

	void LoadFile(const std::string& file_name, std::string& buffer);

};

