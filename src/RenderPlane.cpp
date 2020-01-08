//
// Created by Christian Navolskyi on 06.01.20.
//

#include "RenderPlane.h"

RenderPlane::RenderPlane(int width, int height) : planeWidth(width), planeHeight(height)
{
	glGenVertexArrays(1, &vertexArrayObjectId);
	glGenTextures(1, &textureId);
	glGenBuffers(1, &vertexBufferObjectId);
	glGenBuffers(1, &textureBufferObjectId);
	glGenBuffers(1, &pixelBufferObject);
}

RenderPlane::~RenderPlane()
{
	glDeleteBuffers(1, &vertexBufferObjectId);
	glDeleteBuffers(1, &textureBufferObjectId);
	glDeleteVertexArrays(1, &vertexArrayObjectId);
	glDeleteTextures(1, &textureId);
}

void RenderPlane::init()
{
	glBindVertexArray(vertexArrayObjectId);

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, texturePixels);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjectId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, textureBufferObjectId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

	updateSize(planeWidth, planeHeight);
}

void RenderPlane::updateSize(int width, int height)
{
	planeWidth = width;
	planeHeight = height;

	textureCoords[1] = planeHeight - 1;

	textureCoords[4] = planeWidth - 1;
	textureCoords[5] = planeHeight - 1;

	textureCoords[6] = planeWidth - 1;

//	glBindTexture(GL_TEXTURE_2D, textureId);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, planeWidth, planeHeight, 0, GL_RGB, GL_FLOAT, nullptr);
}
