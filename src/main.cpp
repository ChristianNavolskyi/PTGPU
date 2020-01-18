
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


struct ReferenceHolder
{
	Renderer *renderer;
	CLTracer *tracer;
	Scene *scene;
	InteractiveCamera *camera;
	int currentSceneId;
	bool mousePressed;
	bool showToolTip;
};

void setupCallbacks(GLFWwindow *window, ReferenceHolder *holder);

static void printError(const char *description)
{
	std::cerr << description << std::endl;
}

Scene *getCornellBoxSceneSingleAreaLight(InteractiveCamera *camera)
{
	Scene *scene = new Scene();

	scene->addSphere(200.f, glm::vec3(201.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f)); // right wall
	scene->addSphere(200.f, glm::vec3(-201.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f)); // left wall
	scene->addSphere(200.f, glm::vec3(0.f, 0.f, -201.f), glm::vec3(1.f, 1.f, 1.f)); // front wall
	scene->addSphere(200.f, glm::vec3(0.f, 0.f, 210.f), glm::vec3(1.f, 1.f, 1.f)); // back wall
	scene->addSphere(200.f, glm::vec3(0.f, -201.f, 0.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), 0.4f, 0.6f); // floor
	scene->addSphere(200.f, glm::vec3(0.f, 201.f, 0.f), glm::vec3(1.f, 1.f, 1.f)); // ceiling

	scene->addSphere(1.f, glm::vec3(0.f, 1.8f, 0.f), glm::vec3(0.f), glm::vec3(10.f)); // light source

	scene->addSphere(0.2f, glm::vec3(-0.7f, -0.8f, 0.f), glm::vec3(0.8f, 0.3f, 0.1f), glm::vec3(0.f), 0.1f, 0.9f); // red sphere
	scene->addSphere(0.2f, glm::vec3(0.7f, -0.8f, 0.f), glm::vec3(1.f), glm::vec3(0.f), 1.f); // white sphere
	scene->addSphere(0.2f, glm::vec3(0.f, -0.8f, 0.f), glm::vec3(0.3f, 0.4f, 0.9f), glm::vec3(0.f), 0.4f, 0.6f); // blue sphere

	camera->centerPosition = glm::vec3(-0.385f, 0.102f, 2.001f);
	camera->yaw = -0.418f;
	camera->pitch = 0.533f;

	return scene;
}

Scene *getCornellBoxSceneSinglePointLight(InteractiveCamera *camera)
{
	Scene *scene = new Scene();

	scene->addSphere(200.f, glm::vec3(201.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f)); // right wall
	scene->addSphere(200.f, glm::vec3(-201.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f)); // left wall
	scene->addSphere(200.f, glm::vec3(0.f, 0.f, -201.f), glm::vec3(1.f, 1.f, 1.f)); // back wall
	scene->addSphere(200.f, glm::vec3(0.f, -201.f, 0.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), 0.4f, 0.6f); // floor
	scene->addSphere(200.f, glm::vec3(0.f, 201.f, 0.f), glm::vec3(1.f, 1.f, 1.f)); // ceiling

	scene->addSphere(0.f, glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec3(1.0f, 0.3f, 0.3f)); // point light source

	scene->addSphere(0.2f, glm::vec3(-0.7f, -0.8f, 0.f), glm::vec3(0.8f, 0.3f, 0.1f), glm::vec3(0.f), 0.1f, 0.9f); // red sphere
	scene->addSphere(0.2f, glm::vec3(0.7f, -0.8f, 0.f), glm::vec3(1.f), glm::vec3(0.f), 1.f); // white sphere
	scene->addSphere(0.2f, glm::vec3(0.f, -0.8f, 0.f), glm::vec3(0.3f, 0.4f, 0.9f), glm::vec3(0.f), 0.4f, 0.6f); // blue sphere

	camera->centerPosition = glm::vec3(0.f, 0.f, 3.f);
	camera->yaw = 0.f;
	camera->pitch = 0.05f;

	return scene;
}

Scene *getCornellBoxSceneWithMultipleLights(InteractiveCamera *camera)
{
	Scene *scene = new Scene();

	scene->addSphere(200.f, glm::vec3(201.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f)); // right wall
	scene->addSphere(200.f, glm::vec3(-201.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f)); // left wall
	scene->addSphere(200.f, glm::vec3(0.f, 0.f, -201.f), glm::vec3(1.f, 1.f, 1.f)); // back wall
	scene->addSphere(200.f, glm::vec3(0.f, -201.f, 0.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), 0.4f, 0.6f); // floor
	scene->addSphere(200.f, glm::vec3(0.f, 201.f, 0.f), glm::vec3(1.f, 1.f, 1.f)); // ceiling

	scene->addSphere(1.f, glm::vec3(0.f, 1.8f, 0.f), glm::vec3(0.f), glm::vec3(0.75f)); // light source
	scene->addSphere(0.f, glm::vec3(-0.8f, -0.75f, 0.5f), glm::vec3(0.f), glm::vec3(1.f, 0.3f, 0.3f)); // point light source
	scene->addSphere(0.f, glm::vec3(0.7f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec3(0.3f, 1.f, 0.3f)); // point light source
	scene->addSphere(0.f, glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec3(1.0f, 0.3f, 0.3f)); // point light source

	scene->addSphere(0.2f, glm::vec3(-0.7f, -0.8f, 0.f), glm::vec3(0.8f, 0.3f, 0.1f), glm::vec3(0.f), 0.1f, 0.9f); // red sphere
	scene->addSphere(0.2f, glm::vec3(0.7f, -0.8f, 0.f), glm::vec3(1.f), glm::vec3(0.f), 1.f); // white sphere
	scene->addSphere(0.2f, glm::vec3(0.f, -0.8f, 0.f), glm::vec3(0.3f, 0.4f, 0.9f), glm::vec3(0.f), 0.4f, 0.6f); // blue sphere

	camera->centerPosition = glm::vec3(0.f, 0.f, 3.f);
	camera->yaw = 0.f;
	camera->pitch = 0.05f;

	return scene;
}

Scene *getUniverseScene(InteractiveCamera *camera)
{
	Scene *scene = new Scene();

	scene->setBackgroundColor(0.f, 0.f, 0.f);

	scene->addSphere(0.75f, glm::vec3(0.f), glm::vec3(0.f), glm::vec3(10.f)); // sun

	scene->addSphere(0.2f, glm::vec3(-1.1f, -0.1f, 0.1f), glm::vec3(0.8f, 0.4f, 0.3f)); // mercury
	scene->addSphere(0.5f, glm::vec3(-2.f, -0.2f, 0.4f), glm::vec3(0.9f, 0.3f, 0.1f)); // venus
	scene->addSphere(0.3f, glm::vec3(-5.f, 1.f, 0.f), glm::vec3(0.2f, 0.2f, 0.9f)); // earth
	scene->addSphere(0.1f, glm::vec3(-4.8f, 0.5f, 0.f), glm::vec3(1.f, 1.f, 1.f)); // earth
	scene->addSphere(0.25f, glm::vec3(-6.f, 2.f, 0.5f), glm::vec3(0.6f, 0.4f, 0.2f)); // mars

	camera->centerPosition = glm::vec3(1.12f, -0.12f, 1.73f);
	camera->yaw = 1.185;
	camera->pitch = -0.131;

	return scene;
}

void showImGuiToolTip(ReferenceHolder *holder)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	glm::vec3 position = holder->camera->centerPosition;
	glm::ivec2 resolution = holder->camera->resolution;
	glm::vec3 view = holder->camera->viewDirection;

	float fps = holder->tracer->getFPS();

	ImGui::Begin("Scene info");

	ImGui::Text("Camera Position: (%f, %f, %f)", position.x, position.y, position.z);
	ImGui::Text("Camera Yaw: %f", holder->camera->yaw);
	ImGui::Text("Camera Pitch: %f", holder->camera->pitch);
	ImGui::Text("Camera View Direction: (%f, %f, %f)", view.x, view.y, view.z);
	ImGui::Text("Camera Resolution: (%d, %d)", resolution.x, resolution.y);

	ImGui::Text("Iteration: %d", holder->tracer->iteration);
	ImGui::SliderInt("Max Samples", &holder->tracer->maxSamples, 1, 10000);

	ImGui::Text("FPS: %f", fps);

	ImGui::Separator();
	ImGui::Text("Render Options");

	if (ImGui::Button("Default"))
	{
		holder->tracer->setRenderOption(DEFAULT);
	}
	ImGui::SameLine();
	if (ImGui::Button("Normal"))
	{
		holder->tracer->setRenderOption(NORMAL);
	}
	ImGui::SameLine();
	if (ImGui::Button("Depth"))
	{
		holder->tracer->setRenderOption(DEPTH);
	}
	ImGui::SameLine();
	if (ImGui::Button("Color"))
	{
		holder->tracer->setRenderOption(COLOR);
	}
	ImGui::SameLine();
	if (ImGui::Button("Emittance"))
	{
		holder->tracer->setRenderOption(EMITTANCE);
	}
	ImGui::SameLine();
	if (ImGui::Button("Sphere Id"))
	{
		holder->tracer->setRenderOption(SPHERE_ID);
	}
	ImGui::SameLine();
	if (ImGui::Button("Random Numbers"))
	{
		holder->tracer->setRenderOption(RANDOM);
	}

	const char *sceneNames[] = {"Cornell Box Area Light", "Cornell Box Point Light", "Cornell Box (Mixed Lights)", "Universe"};
	ImGui::Separator();
	if (ImGui::ListBox("Scenes", &holder->currentSceneId, sceneNames, 4))
	{
		delete holder->scene;

		switch (holder->currentSceneId)
		{
		case 0:
			holder->scene = getCornellBoxSceneSingleAreaLight(holder->camera);
			break;
		case 1:
			holder->scene = getCornellBoxSceneSinglePointLight(holder->camera);
			break;
		case 2:
			holder->scene = getCornellBoxSceneWithMultipleLights(holder->camera);
			break;
		case 3:
			holder->scene = getUniverseScene(holder->camera);
			break;
		default:
			holder->scene = getCornellBoxSceneWithMultipleLights(holder->camera);
			break;
		}

		holder->scene->setCamera(holder->camera);
		holder->scene->linkUpdateListener(holder->tracer);
		holder->tracer->changeScene(holder->scene);
	}

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

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	size_t localWorkSize[] = {16, 16};

	auto *camera = new InteractiveCamera(width, height);
	Scene *scene = getCornellBoxSceneSingleAreaLight(camera);
	scene->setCamera(camera);

	Renderer renderer(width, height);
	renderer.init("../src/shaders/shader.vert", "../src/shaders/shader.frag");

	glFinish();

	CLTracer tracer(scene, localWorkSize);
	tracer.init("../src/kernels/pathtracer.cl", renderer.getGLTextureReference());

	ReferenceHolder holder{};
	holder.renderer = &renderer;
	holder.tracer = &tracer;
	holder.scene = scene;
	holder.camera = camera;
	holder.mousePressed = false;
	holder.showToolTip = true;

	setupCallbacks(window, &holder);

	do
	{
		tracer.trace();
		renderer.render();

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

void setupCallbacks(GLFWwindow *window, ReferenceHolder *holder)
{
	auto frameSizeCallback = [](GLFWwindow *window, int width, int height)
	{
		auto holder = (ReferenceHolder *)
				glfwGetWindowUserPointer(window);

		holder->renderer->setRenderSize(width, height);
		holder->scene->changeResolution(width, height);
	};

	auto mouseButtonCallback = [](GLFWwindow *window, int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			auto holder = (ReferenceHolder *) glfwGetWindowUserPointer(window);

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
		auto holder = (ReferenceHolder *) glfwGetWindowUserPointer(window);

		if (holder->mousePressed)
		{
			holder->scene->updateMousePosition((float) xPos, (float) yPos);
		}
	};

	auto keyCallback = [](GLFWwindow *window, int key, int scanCode, int action, int mods)
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			auto holder = ((ReferenceHolder *) glfwGetWindowUserPointer(window));
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
			case GLFW_KEY_SPACE:
				holder->tracer->resetRendering();
				break;
			default:
				break;
			}
		}
	};

	glfwSetWindowUserPointer(window, holder);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseMovementCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetFramebufferSizeCallback(window, frameSizeCallback);
}
