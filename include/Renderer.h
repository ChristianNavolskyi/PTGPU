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
	GLuint vertexBufferId;
	GLuint colorBufferId;

public:
	Renderer(int width, int height);

	~Renderer();

	void init(const char *vertexShaderPath, const char *fragmentShaderPath);

	void render();

	void setRenderSize(int width, int height);

	GLuint getVertexBufferId();

	GLuint getColorBufferId();

	void allocateBuffers();
};

