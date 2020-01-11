#version 330 core

uniform sampler2D imageTexture;
in vec2 tex_coord;

out vec4 color;

void main() {
	color = vec4(texture(imageTexture, tex_coord).rgb, 1.0);
}