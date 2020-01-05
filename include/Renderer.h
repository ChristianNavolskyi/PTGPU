//
// Created by Melina Christian Navolskyi on 05.01.20.
//

#pragma once

#include <GL/glew.h>


#ifndef DEBUG
#define DEBUG false
#endif

class Renderer
{
private:
	GLuint programId;

	GLuint vertexArrayObjectId;
	GLuint vertexBufferObjectId;

	GLuint textureId;
	GLuint textureBufferObjectId;

	GLfloat texturePixels[12] = {
			0.f, 0.f, 0.f, 1.f, 1.f, 1.f,
			1.f, 1.f, 1.f, 0.f, 0.f, 0.f
	};

	GLfloat textureCoords[8] = {
			0.f, 1.f,
			0.f, 0.f,
			1.f, 1.f,
			1.f, 0.f
	};

#if DEBUG
	const GLfloat vertices[12] = {
			-.5f, .5f, 0.f, // top left
			-.5f, -.5f, 0.f, // bottom left
			.5f, .5f, 0.f, // top right
			.5f, -.5f, 0.f  // bottom left
	};
#else
	const GLfloat vertices[12] = {
			-1.f, 1.f, 0.f, // top left
			-1.f, -1.f, 0.f, // bottom left
			1.f, 1.f, 0.f, // top right
			1.f, -1.f, 0.f  // bottom left
	};
#endif

	void loadDataToGPU();

	void setShaderArgs();

public:
	Renderer();

	~Renderer();

	void init(const char *vertexShaderPath, const char *fragmentShaderPath);

	void render();
};

