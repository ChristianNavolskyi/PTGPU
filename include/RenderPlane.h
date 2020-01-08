//
// Created by Christian Navolskyi on 06.01.20.
//

#pragma once

#include <GL/glew.h>

#ifndef DEBUG
#define DEBUG false
#endif

class RenderPlane
{
private:
	int planeWidth, planeHeight;
	GLuint vertexArrayObjectId;
	GLuint pixelBufferObject;

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


public:
	GLuint vertexBufferObjectId;

	GLuint textureId;
	GLuint textureBufferObjectId;

	RenderPlane(int width, int height);

	~RenderPlane();

	void init();

	void updateSize(int width, int height);

	int getWidth()
	{
		return planeWidth;
	}

	int getHeight()
	{
		return planeHeight;
	}
};


