#include <fstream>
#include <iostream>
#include <GL/glew.h>

#include "Renderer.h"


std::string loadFileToString(const char *filepath)
{
	std::string result;
	std::ifstream stream(filepath, std::ios::in);

	if (stream.is_open())
	{
		std::string currentLine;

		while (getline(stream, currentLine))
		{
			result += currentLine + "\n";
		}

		stream.close();
	}

	return result;
}

GLuint addShader(const char *shaderPath, GLenum shaderType, GLuint programId)
{
	GLuint shaderId = glCreateShader(shaderType);
	std::string shaderCode = loadFileToString(shaderPath);
	const char *rawShaderCode = shaderCode.c_str();

	glShaderSource(shaderId, 1, &rawShaderCode, nullptr);
	glCompileShader(shaderId);
	GLint success;

	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(shaderId, 512, nullptr, infoLog);

		std::cout << "Compilation of shader " << shaderPath << " failed: " << infoLog << std::endl;
	}

	glAttachShader(programId, shaderId);

	return shaderId;
}

GLuint loadShaders(const char *vertexShaderPath, const char *fragmentShaderPath)
{
	GLuint programId = glCreateProgram();
	GLuint vertexShaderId = addShader(vertexShaderPath, GL_VERTEX_SHADER, programId);
	GLuint fragmentShaderId = addShader(fragmentShaderPath, GL_FRAGMENT_SHADER, programId);

	glLinkProgram(programId);

	GLint success;
	glGetProgramiv(programId, GL_LINK_STATUS, &success);

	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(programId, 512, nullptr, infoLog);

		std::cout << "Linking of OpenGL program failed: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	return programId;
}

Renderer::Renderer(int width, int height) : programId(0), width(width), height(height)
{
}

Renderer::~Renderer()
{
	glDeleteProgram(programId);
	glDeleteBuffers(1, &vertexArrayId);
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteBuffers(1, &textureCoordinateBufferId);
	glDeleteBuffers(1, &pixelBufferId);
	glDeleteTextures(1, &textureId);
}

void Renderer::init(const char *vertexShaderPath, const char *fragmentShaderPath)
{
	programId = loadShaders(vertexShaderPath, fragmentShaderPath);

	glUseProgram(programId);

	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);

	GLint attributeLocation;

	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	attributeLocation = glGetAttribLocation(programId, "POSITION");
	glVertexAttribPointer(attributeLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(attributeLocation);

	glGenBuffers(1, &textureCoordinateBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, textureCoordinateBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);
	attributeLocation = glGetAttribLocation(programId, "TEXTURE_COORDINATE");
	glVertexAttribPointer(attributeLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(attributeLocation);

	glGenBuffers(1, &pixelBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, pixelBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * width * height, nullptr, GL_DYNAMIC_DRAW);

	glGenTextures(1, &textureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_FLOAT, texturePixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glUniform1i(glGetUniformLocation(programId, "imageTexture"), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	setRenderSize(width, height);
}

void Renderer::updateTextureCoords()
{
	textureCoords[0] = (float) -width;
	textureCoords[1] = (float) -height;

	textureCoords[2] = (float) -width;
	textureCoords[3] = (float) height - 1.f;

	textureCoords[4] = (float) width - 1.f;
	textureCoords[5] = (float) -height;

	textureCoords[6] = (float) width - 1.f;
	textureCoords[7] = (float) height - 1.f;
}

void Renderer::render()
{
	glBindVertexArray(vertexArrayId);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBufferId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, nullptr);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);

	glFinish();
}

void Renderer::setRenderSize(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;

	updateTextureCoords();

	glBindBuffer(GL_ARRAY_BUFFER, textureCoordinateBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, pixelBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * width * height, nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Renderer::getGLTextureReference()
{
	return pixelBufferId;
}
