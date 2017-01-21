#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D tex;
uniform float fadeout;

void main()
{
	color = texture(tex, uv).rgba * fadeout;
}
