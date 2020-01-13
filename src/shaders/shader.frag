#version 330 core

uniform sampler2D imageTexture;
in vec2 tex_coord;

out vec4 color;

void main() {
	color = vec4(texture(imageTexture, tex_coord).rgb, 1.0);
	//		color = vec4(tex_coord.x / 2000.0, tex_coord.y / 1200.0, 0.0, 1.0);
}