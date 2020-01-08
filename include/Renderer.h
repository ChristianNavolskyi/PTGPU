//
// Created by Christian Navolskyi on 05.01.20.
//

#pragma once

#include <GL/glew.h>
#include <OpenCL/opencl.h>
#include "RenderPlane.h"


class Renderer
{
private:
	GLuint programId;

	RenderPlane *plane;

	void setShaderArgs();

public:
	Renderer(int width, int height);

	~Renderer();

	void init(const char *vertexShaderPath, const char *fragmentShaderPath);

	void render(float *imageData);

	void setRenderSize(int width, int height)
	{
		plane->updateSize(width, height);
	}

	GLenum getTextureTarget()
	{
		return GL_TEXTURE_2D;
	}

	GLuint getTextureId()
	{
		return plane->textureId;
	}
};

