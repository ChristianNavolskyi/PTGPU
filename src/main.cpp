
#include <imgui/imgui.h>
#include <cstdio>
#include <string>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#ifndef DEBUG
#define DEBUG false
#endif

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

static void printError(const char *description)
{
	std::cerr << description << std::endl;
}

int main(int, char **)
{
	// Setup window
	glfwSetErrorCallback([](int error, const char *description)
						 { fprintf(stderr, "Glfw Error %d: %s\n", error, description); });
	if (!glfwInit())
	{
		printError("Failed to initialise GLFW.");
		return -1;
	}

	const char *glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac

	// Create window with graphics context
	GLFWwindow *window = glfwCreateWindow(1000, 600, "OpenCL Path Tracer", nullptr, nullptr);
	if (!window)
	{
		printError("Failed to create window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		printError("Failed to initialise GLEW.");
		glfwTerminate();
		return -1;
	}

	// TODO figure out how to draw the texture to the glfw background
	GLuint programId = loadShaders("../src/shaders/shader.vert", "../src/shaders/shader.frag");

	GLuint vaoID;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	GLfloat texturePixels[] = {
			0.f, 0.f, 0.f, 1.f, 1.f, 1.f,
			1.f, 1.f, 1.f, 0.f, 0.f, 0.f
	};

	GLfloat textureCoords[] = {
			0.f, 1.f,
			0.f, 0.f,
			1.f, 1.f,
			1.f, 0.f
	};

	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, texturePixels);
	glBindTexture(GL_TEXTURE_2D, 0);


#if DEBUG
	static const GLfloat vertices[] = {
			-.5f, .5f, 0.f, // top left
			-.5f, -.5f, 0.f, // bottom left
			.5f, .5f, 0.f, // top right
			.5f, -.5f, 0.f  // bottom left
	};
#else
	static const GLfloat vertices[] = {
			-1.f, 1.f, 0.f, // top left
			-1.f, -1.f, 0.f, // bottom left
			1.f, 1.f, 0.f, // top right
			1.f, -1.f, 0.f  // bottom left
	};
#endif

	GLuint vboID, textureVboId;
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &textureVboId);
	glBindBuffer(GL_ARRAY_BUFFER, textureVboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);


	// vertex positions
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// texture coordinates
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, textureVboId);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	float diff = 0.01f;

	do
	{
		glClear(GL_COLOR_BUFFER_BIT);

		if (texturePixels[0] > 1.f || texturePixels[0] < 0.f)
		{
			diff *= -1.f;
		}

		texturePixels[0] += diff;


		// texture
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, texturePixels);
		GLuint textureLocation = glGetUniformLocation(programId, "tex");
		glUniform1i(textureLocation, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);

		glUseProgram(programId);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (!glfwWindowShouldClose(window));

	glfwTerminate();
	return 0;


	// TODO get keyboard callbacks
	// TODO OpenCL setup
}
