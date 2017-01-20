#include "ShaderProgram.h"

#include <vector>

#include "utility/Log.h"
#include "utility/FileUtility.h"


ShaderProgram::ShaderProgram(const std::string& vert_path, const std::string& frag_path)
{
	//load shaders
	GLuint vert_shader, frag_shader;
	std::string vert_buffer, frag_buffer;

	glUseProgram(0);
	program = glCreateProgram();

	LoadFileToBuffer(vert_path, vert_buffer);
	vert_shader = glCreateShader(GL_VERTEX_SHADER);
	const char* vert_code = vert_buffer.c_str();
	glShaderSource(vert_shader, 1, &vert_code, nullptr);
	glCompileShader(vert_shader);

	GLint isCompiled = 0;
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &maxLength);
		std::string error_message;
		error_message.reserve(maxLength + 1);
		glGetShaderInfoLog(vert_shader, maxLength, NULL, &error_message[0]);
		Logger::Log("Shader vert error: ", std::string(&error_message[0]));
	}

	glAttachShader(program, vert_shader);

	LoadFileToBuffer(frag_path, frag_buffer);
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* frag_code = frag_buffer.c_str();
	glShaderSource(frag_shader, 1, &frag_code, nullptr);
	glCompileShader(frag_shader);

	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &maxLength);
		std::string error_message;
		error_message.reserve(maxLength + 1);
		glGetShaderInfoLog(frag_shader, maxLength, NULL, &error_message[0]);
		Logger::Log("Shader frag error: ", std::string(&error_message[0]));
	}

	glAttachShader(program, frag_shader);

	glLinkProgram(program);




	//after link program
	GLint result = GL_FALSE;
	int info_length;

	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_length);
	if (info_length > 0)
	{
		std::string error_message;
		error_message.reserve(info_length + 1);
		glGetProgramInfoLog(program, info_length, NULL, &error_message[0]);
		Logger::Log("Shader program error: ", std::string(&error_message[0]));
	}



	//clean
	glDetachShader(program, vert_shader);
	glDetachShader(program, frag_shader);

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);
}

ShaderProgram::~ShaderProgram()
{
}

void ShaderProgram::UseProgram()
{
	glUseProgram(program);
}

GLuint ShaderProgram::GetProgram()
{
	return program;
}