#version 330 core

in vec2 POSITION;
in vec2 TEXTURE_COORDINATE;

out vec2 tex_coord;

void main()
{
	gl_Position = vec4(POSITION.xy, 0.0, 1.0);
	tex_coord = TEXTURE_COORDINATE;
}