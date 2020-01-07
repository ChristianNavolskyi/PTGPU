
#include <imgui/imgui.h>
#include <cstdio>
#include <string>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Renderer.h>
#include <CLTracer.h>


std::string loadFileToString(const char *filepath);

GLuint addShader(const char *shaderPath, GLenum shaderType, GLuint programId);

GLuint loadShaders(const char *vertexShaderPath, const char *fragmentShaderPath);

static void printError(const char *description)
{
	std::cerr << description << std::endl;
}

struct SizeAdaptionHolder
{
	Renderer *renderer;
	CLTracer *tracer;
	float *imageData;
};

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

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		printError("Failed to initialise GLEW.");
		glfwTerminate();
		return -1;
	}

	Renderer renderer(width, height);
	renderer.init("../src/shaders/shader.vert", "../src/shaders/shader.frag");

	float *imageData = (float *) malloc(sizeof(float) * 3 * width * height);
	size_t localWorkSize[] = {16, 16};

	glFinish();

	CLTracer tracer(localWorkSize);
	tracer.init("../src/kernels/pathtracer.cl", "fill");
//	tracer.addGLTexture(renderer.getTextureTarget(), renderer.getTextureId());
	tracer.setImageSize(width, height);

	SizeAdaptionHolder holder{};
	holder.renderer = &renderer;
	holder.tracer = &tracer;
	holder.imageData = imageData;

	glfwSetWindowUserPointer(window, &holder);
	auto callback = [](GLFWwindow *window, int width, int height)
	{
		auto holder = (SizeAdaptionHolder *)
				glfwGetWindowUserPointer(window);

		free(holder->imageData);
		holder->imageData = (float *) malloc(sizeof(float) * 3 * width * height);
		holder->renderer->setRenderSize(width, height);
		holder->tracer->setImageSize(width, height);
	};
	glfwSetFramebufferSizeCallback(window, callback);

	do
	{
		tracer.trace(holder.imageData);
		renderer.render(holder.imageData);

		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (!glfwWindowShouldClose(window));

	glfwTerminate();
	return 0;


	// TODO get keyboard callbacks
	// TODO OpenCL setup
}
