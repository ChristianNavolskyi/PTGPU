#version 330 core

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXTURE_COORDS;

out vec2 tex_coord;

void main()
{
	gl_Position = vec4(POSITION, 1.0);
	tex_coord = TEXTURE_COORDS;
}