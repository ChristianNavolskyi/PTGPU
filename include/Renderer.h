//
// Created by Christian Navolskyi on 05.01.20.
//

#pragma once

class Renderer
{
private:
	int width, height;

	GLuint programId;
	GLuint vertexArrayId;
	GLuint vertexBufferId;
	GLuint pixelBufferId;
	GLuint textureCoordinateBufferId;
	GLuint textureId;

	GLfloat vertices[8] = {
			-1.f, 1.f,  // tl
			-1.f, -1.f, // bl
			1.f, 1.f,   // tr
			1.f, -1.f,  // br
	};

	GLfloat textureCoords[8];

	GLfloat texturePixels[16] = {
			1.f, 0.f, 0.f, 1.f, // tl
			0.f, 1.f, 0.f, 1.f, // tr
			0.f, 0.f, 1.f, 1.f, // bl
			1.f, 1.f, 1.f, 1.f, // br
	};

public:
	Renderer(int width, int height);

	~Renderer();

	void init(const char *vertexShaderPath, const char *fragmentShaderPath);

	void render(float *imageData);

	void setRenderSize(int newWidth, int newHeight);

	GLuint getGLTextureReference();

	void updateTextureCoords();
};

