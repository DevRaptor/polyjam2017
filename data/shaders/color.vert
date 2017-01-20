#version 330 core
layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec3 vertex_color;

out vec3 fragment_color;

uniform mat4 mvp;
uniform mat4 transform;

void main()
{
	gl_Position = mvp * transform * vec4(vertex_pos, 1);
	fragment_color = vertex_color;
}