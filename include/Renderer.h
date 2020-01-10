//
// Created by Christian Navolskyi on 05.01.20.
//

#pragma once

#include <GL/glew.h>
#include <OpenCL/opencl.h>


class Renderer
{
private:
	int width, height;

	GLuint programId;
	GLuint vertexArrayObjectId;
	GLuint pixelBufferObject;
	GLuint vertexBufferObjectId;
	GLuint textureId;
	GLuint textureBufferObjectId;

	GLfloat texturePixels[12] = {
			0.f, 0.f, 0.f, 1.f, 1.f, 1.f,
			1.f, 1.f, 1.f, 0.f, 0.f, 0.f
	};

	GLfloat textureCoords[8] = {
			0.f, 0.f,
			0.f, 1.f,
			1.f, 0.f,
			1.f, 1.f
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

	void setShaderArgs();

public:
	Renderer(int width, int height);

	~Renderer();

	void init(const char *vertexShaderPath, const char *fragmentShaderPath);

	void render(float *imageData);

	void setRenderSize(int width, int height);

	GLuint getRenderTargetId();
};

