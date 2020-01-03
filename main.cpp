
#include <iostream>

#include "ext/imgui/imgui.h"
#include <GL/glew.h>

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

int main()
{
	std::cout << "Hello, World!" << std::endl;

	GLuint tex;
	glGenTextures(1, &tex);

	return 0;
}