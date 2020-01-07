//
// Created by Melina Christian Navolskyi on 05.01.20.
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

Renderer::Renderer(int width, int height) : programId(0)
{
	plane = new RenderPlane(width, height);
}

Renderer::~Renderer()
{
	glDeleteProgram(programId);
}

void Renderer::setShaderArgs()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, plane->vertexBufferObjectId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, plane->textureBufferObjectId);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// texture
	glBindTexture(GL_TEXTURE_2D, plane->textureId);
	GLuint textureLocation = glGetUniformLocation(programId, "tex");
	glUniform1i(textureLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, plane->textureId);
}

void Renderer::init(const char *vertexShaderPath, const char *fragmentShaderPath)
{
	plane->init();
	programId = loadShaders(vertexShaderPath, fragmentShaderPath);

	glUseProgram(programId);

	setShaderArgs();
}

void Renderer::render(float* imageData)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, plane->textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, plane->getWidth(), plane->getHeight(), 0, GL_RGB, GL_FLOAT, imageData);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glFinish();
}

