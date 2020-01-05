#version 330 core

uniform sampler2D tex;

in vec2 tex_coord;

out vec4 color;

void main() {
	color = vec4(texture(tex, tex_coord).xyz, 1.f);
}