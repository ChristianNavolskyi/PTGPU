
#include <iostream>

#include <OpenGL/gl3.h>

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

int main()
{
	std::cout << "Hello, World!" << std::endl;

	GLuint textureReference;
	glGenTextures(1, &textureReference);




	return 0;
}