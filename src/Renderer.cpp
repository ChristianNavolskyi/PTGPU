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

Renderer::Renderer()
{
	glGenVertexArrays(1, &vertexArrayObjectId);
	glGenTextures(1, &textureId);
	glGenBuffers(1, &vertexBufferObjectId);
	glGenBuffers(1, &textureBufferObjectId);
}

Renderer::~Renderer()
{
	glDeleteBuffers(1, &vertexBufferObjectId);
	glDeleteBuffers(1, &textureBufferObjectId);
	glDeleteVertexArrays(1, &vertexArrayObjectId);
	glDeleteTextures(1, &textureId);
	glDeleteProgram(programId);
}

void Renderer::loadDataToGPU()
{
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, texturePixels);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjectId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, textureBufferObjectId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);
}


void Renderer::setShaderArgs()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjectId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, textureBufferObjectId);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// texture
	glBindTexture(GL_TEXTURE_2D, textureId);
	GLuint textureLocation = glGetUniformLocation(programId, "tex");
	glUniform1i(textureLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

void Renderer::init(const char *vertexShaderPath, const char *fragmentShaderPath)
{
	programId = loadShaders(vertexShaderPath, fragmentShaderPath);

	glUseProgram(programId);

	glBindVertexArray(vertexArrayObjectId);

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	loadDataToGPU();
	setShaderArgs();
}

void Renderer::render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
