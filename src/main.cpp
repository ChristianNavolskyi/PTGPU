
#include <imgui/imgui.h>
#include <cstdio>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Renderer.h>
#include <CLTracer.h>
#include <Scene.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


struct GLFWReferenceHolder
{
	Renderer *renderer;
	CLTracer *tracer;
	Scene *scene;
	float *imageData;
	bool mousePressed;
	bool showToolTip;
};

void setupCallbacks(GLFWwindow *window, GLFWReferenceHolder *holder);

static void printError(const char *description)
{
	std::cerr << description << std::endl;
}

void showImGuiToolTip(GLFWReferenceHolder *holder)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	glm::vec3 position = holder->scene->camera.centerPosition;
	glm::ivec2 resolution = holder->scene->camera.resolution;

	ImGui::Begin("Scene info");

	ImGui::Text("Camera Position: (%f, %f, %f)", position.x, position.y, position.z);
	ImGui::Text("Camera Yaw: %f", holder->scene->camera.yaw);
	ImGui::Text("Camera Pitch: %f", holder->scene->camera.pitch);
	ImGui::Text("Camera Resolution: (%d, %d)", resolution.x, resolution.y);
	ImGui::Text("Iteration: %d", holder->tracer->iteration);

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

	const char *glsl_version = "#version 330";
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

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void) io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();


	size_t localWorkSize[] = {16, 16};
	float *imageData = (float *) malloc(sizeof(float) * 4 * width * height);

	Scene scene(width, height);
	scene.addSphere(0.f, -200.4f, 0.f, 200.f, 0.9f, 0.3f, 0.f, 0.f, 0.f, 0.f); // floor
	scene.addSphere(-0.25f, -0.24f, -0.1f, 0.16f, 0.8f, 0.7f, 0.6f, 0.f, 0.f, 0.f); // left
	scene.addSphere(0.25f, -0.24f, 0.1f, 0.16f, 0.8f, 0.7f, 0.6f, 0.f, 0.f, 0.f); // right
	scene.addSphere(0.f, 1.36f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.9f, 0.8f, 0.6f); // light
//	scene.addSphere(1.f, 1.f, -20.f, 2.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f);

	Renderer renderer(width, height);
	renderer.init("../src/shaders/shader.vert", "../src/shaders/shader.frag");

	glFinish();

	CLTracer tracer(&scene, localWorkSize);
	tracer.init("../src/kernels/pathtracer.cl", renderer.getGLTextureReference());

	GLFWReferenceHolder holder{};
	holder.renderer = &renderer;
	holder.tracer = &tracer;
	holder.scene = &scene;
	holder.imageData = imageData;
	holder.mousePressed = false;
	holder.showToolTip = true;

	tracer.clearImage(holder.imageData);
	setupCallbacks(window, &holder);

	do
	{
		tracer.updateCamera();
		tracer.trace(holder.imageData);
		renderer.render(holder.imageData);

		if (holder.showToolTip)
		{
			showImGuiToolTip(&holder);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (!glfwWindowShouldClose(window));

	glfwTerminate();
	return 0;
}

void setupCallbacks(GLFWwindow *window, GLFWReferenceHolder *holder)
{
	glfwSetWindowUserPointer(window, holder);
	auto frameSizeCallback = [](GLFWwindow *window, int width, int height)
	{
		auto holder = (GLFWReferenceHolder *)
				glfwGetWindowUserPointer(window);

		free(holder->imageData);

		holder->imageData = (float *) malloc(sizeof(float) * 4 * width * height);

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
			auto holder = ((GLFWReferenceHolder *) glfwGetWindowUserPointer(window));
			auto scene = holder->scene;

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
			case GLFW_KEY_T:
				holder->showToolTip = !holder->showToolTip;
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
