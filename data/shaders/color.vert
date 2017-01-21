#version 330 core

layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec2 vertex_uv;

uniform mat4 mvp;
uniform mat4 transform;

out vec2 uv;

void main()
{
	gl_Position = mvp * transform * vec4(vertex_pos, 1);
	uv = vertex_uv;
}
