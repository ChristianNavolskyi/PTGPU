//
// Created by Christian Navolskyi on 07.01.20.
//

#include "Scene.h"
#include "CLUtil.h"

Scene::Scene() : spheres(), sceneInfo()
{
	sceneInfo.sphereCount = 0;
	sceneInfo.lightSphereCount = 0;
	sceneInfo.totalRadiance = 0.f;
	sceneInfo.backgroundColor = {{0.7f, 0.7f, 0.7f}};
}

Scene::~Scene()
{
	spheres.clear();
}

void Scene::setCamera(InteractiveCamera *camera)
{
	this->camera = camera;
}

#define float3(vec3) {{(vec3).x, (vec3).y, (vec3).z}}

void Scene::addSphere(float radius, glm::vec3 position, glm::vec3 color, glm::vec3 emittance, float diffuse, float specular, float transmissive)
{
	glm::vec3 surfaceCharacteristic(diffuse, specular, transmissive);
	surfaceCharacteristic = glm::normalize(surfaceCharacteristic);

	Sphere sphere{};
	sphere.radius = radius;
	sphere.position = float3(position);
	sphere.color = float3(color);
	sphere.emittance = float3(emittance);
	sphere.surfaceCharacteristic = float3(surfaceCharacteristic);

	float radiance;

	if ((radiance = glm::length(emittance)) > FLT_EPSILON)
	{
		LightSphere lightSphere{};

		lightSphere.radiance = radiance;
		lightSphere.sphereId = spheres.size();

		lightSpheres.push_back(lightSphere);

		sceneInfo.lightSphereCount++;
		sceneInfo.totalRadiance += radiance;
	}

	sceneInfo.sphereCount++;

	spheres.push_back(sphere);
}

void Scene::addTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color, glm::vec3 emittance, float diffuse, float specular, float transmissive)
{
	glm::vec3 surfaceCharacteristic(diffuse, specular, transmissive);
	surfaceCharacteristic = glm::normalize(surfaceCharacteristic);

	Triangle triangle{};
	triangle.p1 = float3(p1);
	triangle.p2 = float3(p2);
	triangle.p3 = float3(p3);
	triangle.color = float3(color);
	triangle.emittance = float3(emittance);
	triangle.surfaceCharacteristic = float3(surfaceCharacteristic);

	float radiance;

	if ((radiance = glm::length(emittance)) > FLT_EPSILON)
	{
		LightTriangle lightTriangle{};

		lightTriangle.radiance = radiance;
		lightTriangle.triangleId = triangles.size();

		lightTriangles.push_back(lightTriangle);

		sceneInfo.lightTriangleCount++;
		sceneInfo.totalRadiance += radiance;
	}

	sceneInfo.triangleCount++;

	triangles.push_back(triangle);
}

void Scene::addPlane(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 color, glm::vec3 emittance, float diffuse, float specular, float transmissive)
{
	addTriangle(p1, p2, p3, color, emittance, diffuse, specular, transmissive);
	addTriangle(p2, p3, p4, color, emittance, diffuse, specular, transmissive);
}

void Scene::setBackgroundColor(float r, float g, float b)
{
	sceneInfo.backgroundColor = {{r, g, b}};
}

Camera Scene::getRenderCamera()
{
	return camera->createRenderCamera();
}

void Scene::linkUpdateListener(RenderInfoListener *listener)
{
	changeListener = listener;
}

Sphere *Scene::getSphereData()
{
	return &spheres[0];
}

size_t Scene::getSphereSize()
{
	return sizeof(Sphere) * spheres.size();
}

LightSphere *Scene::getLightSphereData()
{
	return &lightSpheres[0];
}


size_t Scene::getLightSphereSize()
{
	return sizeof(LightSphere) * lightSpheres.size();
}

Triangle *Scene::getTriangleData()
{
	return &triangles[0];
}

size_t Scene::getTriangleSize()
{
	return sizeof(Triangle) * triangles.size();
}

LightTriangle *Scene::getLightTriangleData()
{
	return &lightTriangles[0];
}

size_t Scene::getLightTriangleSize()
{
	return sizeof(LightTriangle) * lightTriangles.size();
}

SceneInfo *Scene::getSceneInfo()
{
	return &sceneInfo;
}

void Scene::changeResolution(int width, int height)
{
	camera->setResolution(width, height);
	notifyListenerResolutionChanged(width, height);
}

glm::ivec2 Scene::getResolution()
{
	return camera->getResolution();
}

void Scene::notifyListenerResolutionChanged(int width, int height)
{
	if (changeListener)
	{
		changeListener->notifySizeChanged(width, height);
	}
}

void Scene::move(SceneMovementDirections direction)
{
	float delta = 0.1f;

	switch (direction)
	{
	case FORWARD:
		camera->goForward(delta);
		break;
	case BACKWARD:
		camera->goForward(-delta);
		break;
	case UP:
		camera->changeAltitude(delta);
		break;
	case DOWN:
		camera->changeAltitude(-delta);
		break;
	case RIGHT:
		camera->strafe(-delta);
		break;
	case LEFT:
		camera->strafe(delta);
		break;
	case YAW_RIGHT:
		camera->changeYaw(delta);
		break;
	case YAW_LEFT:
		camera->changeYaw(-delta);
		break;
	case PITCH_UP:
		camera->changePitch(-delta);
		break;
	case PITCH_DOWN:
		camera->changePitch(delta);
		break;
	}

	notifyListenerCameraChanged();
}

void Scene::initialMousePosition(float xPos, float yPos)
{
	camera->setInitialMousePosition(xPos, yPos);
}

void Scene::updateMousePosition(float xPos, float yPos)
{
	camera->handleMouseMovement(xPos, yPos);

	notifyListenerCameraChanged();
}

void Scene::notifyListenerCameraChanged()
{
	if (changeListener)
	{
		changeListener->notify();
	}
}

