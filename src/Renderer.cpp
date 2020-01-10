//
// Created by Christian Navolskyi on 05.01.20.
//

#include <fstream>
#include <iostream>
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
	glGenBuffers(1, &vertexBufferId);
	glGenBuffers(1, &colorBufferId);
}

Renderer::~Renderer()
{
	glDeleteProgram(programId);
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteBuffers(1, &colorBufferId);
}

void Renderer::init(const char *vertexShaderPath, const char *fragmentShaderPath)
{
	programId = loadShaders(vertexShaderPath, fragmentShaderPath);

	glUseProgram(programId);

	allocateBuffers();
}

void Renderer::render(float *imageData, float *imagePlane)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * width * height, imagePlane, GL_STATIC_DRAW);
	glVertexPointer(2, GL_FLOAT, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, colorBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * width * height, imageData, GL_DYNAMIC_DRAW);
	glColorPointer(3, GL_FLOAT, 0, nullptr);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_POINT, 0, width * height);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glFinish();
}

void Renderer::setRenderSize(int width, int height)
{
	this->width = width;
	this->height = height;

	allocateBuffers();
}

GLuint Renderer::getVertexBufferId()
{
	return vertexBufferId;
}

GLuint Renderer::getColorBufferId()
{
	return colorBufferId;
}

void Renderer::allocateBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * width * height, nullptr, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colorBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * width * height, nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	std::cout << "Allocated OpenGL vertex and color buffer" << std::endl;
}

