
#include <imgui/imgui.h>
#include <cstdio>
#include <string>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Renderer.h>
#include <CLTracer.h>
#include <Scene.h>


struct GLFWReferenceHolder
{
	Renderer *renderer;
	Scene *scene;
	float *imageData;
	float *imagePlane;
	bool mousePressed;
};

void setupCallbacks(GLFWwindow *window, GLFWReferenceHolder *holder);

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
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, GL_FALSE);

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

	size_t localWorkSize[] = {16, 16};
	float *imageData = (float*) malloc(sizeof(float) * 3 * width * height);
	float *imagePlane = (float*) malloc(sizeof(float) * 3 * width * height);

	Scene scene(width, height);
	scene.addSphere(0.f, 1.f, -10.f, 2.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
	scene.addSphere(0.f, 0.f, 200.f, 2.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f);

	Renderer renderer(width, height);
	renderer.init("../src/shaders/shader.vert", "../src/shaders/shader.frag");

	glFinish();

	CLTracer tracer(scene, localWorkSize);
	tracer.init("../src/kernels/pathtracer.cl");
//	tracer.initializeRenderPlane(imagePlane);
//	tracer.linkOpenGLResources(renderer.getVertexBufferId(), renderer.getColorBufferId());

	GLFWReferenceHolder holder{};
	holder.renderer = &renderer;
	holder.scene = &scene;
	holder.imageData = imageData;
	holder.imagePlane = imagePlane;
	holder.mousePressed = false;

	setupCallbacks(window, &holder);

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

void setupCallbacks(GLFWwindow *window, GLFWReferenceHolder *holder)
{
	glfwSetWindowUserPointer(window, holder);
	auto frameSizeCallback = [](GLFWwindow *window, int width, int height)
	{
		auto holder = (GLFWReferenceHolder *)
				glfwGetWindowUserPointer(window);

		free(holder->imagePlane);
		free(holder->imageData);

		holder->imageData = (float*) malloc(sizeof(float) * 3 * width * height);
		holder->imagePlane = (float*) malloc(sizeof(float) * 2 * width * height);

		holder->renderer->setRenderSize(width, height);
		holder->scene->changeResolution(width, height);
	};

	auto mouseButtonCallback = [](GLFWwindow *window, int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			auto holder = (GLFWReferenceHolder *) glfwGetWindowUserPointer(window);

			if (action == GLFW_PRESS)
			{
				double xPos, yPos;
				glfwGetCursorPos(window, &xPos, &yPos);

				holder->scene->initialMousePosition((float) xPos, (float) yPos);
				holder->mousePressed = true;
			}
			else if (action == GLFW_RELEASE)
			{
				holder->mousePressed = false;
			}
		}
	};

	auto mouseMovementCallback = [](GLFWwindow *window, double xPos, double yPos)
	{
		auto holder = (GLFWReferenceHolder *) glfwGetWindowUserPointer(window);

		if (holder->mousePressed)
		{
			holder->scene->updateMousePosition((float) xPos, (float) yPos);
		}
	};

	auto keyCallback = [](GLFWwindow *window, int key, int scanCode, int action, int mods)
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			auto scene = ((GLFWReferenceHolder *) glfwGetWindowUserPointer(window))->scene;

			switch (key)
			{
			case GLFW_KEY_W:
				scene->move(FORWARD);
				break;
			case GLFW_KEY_S:
				scene->move(BACKWARD);
				break;
			case GLFW_KEY_A:
				scene->move(LEFT);
				break;
			case GLFW_KEY_D:
				scene->move(RIGHT);
				break;
			case GLFW_KEY_R:
				scene->move(UP);
				break;
			case GLFW_KEY_F:
				scene->move(DOWN);
				break;
			case GLFW_KEY_LEFT:
				scene->move(YAW_LEFT);
				break;
			case GLFW_KEY_RIGHT:
				scene->move(YAW_RIGHT);
				break;
			case GLFW_KEY_UP:
				scene->move(PITCH_UP);
				break;
			case GLFW_KEY_DOWN:
				scene->move(PITCH_DOWN);
				break;
			default:
				break;
			}
		}
	};

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseMovementCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetFramebufferSizeCallback(window, frameSizeCallback);
}
