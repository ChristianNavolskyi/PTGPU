//
// Created by Christian Navolskyi on 07.01.20.
//

#include "Scene.h"
#include "CLUtil.h"

Scene::Scene(int width, int height) : spheres(), camera(width, height)
{
	camera = InteractiveCamera(width, height);

	sceneInfo.sphereCount = 0;
	sceneInfo.lightSphereCount = 0;
	sceneInfo.totalRadiance = 0.f;
}

Scene::~Scene()
{
	spheres.clear();
}

void Scene::addSphere(float xPos, float yPos, float zPos, float radius, float rColor, float gColor, float bColor, float rEmittance, float gEmittance, float bEmittance, float diffuse, float specular,
					  float transmissive)
{
	glm::vec3 surfaceCharacteristic(diffuse, specular, transmissive);
	surfaceCharacteristic = glm::normalize(surfaceCharacteristic);

	glm::vec3 emittance(rEmittance, gEmittance, bEmittance);

	Sphere sphere{};
	sphere.radius = radius;
	sphere.position = {{xPos, yPos, zPos}};
	sphere.color = {{rColor, gColor, bColor, 1.f}};
	sphere.emittance = {{emittance.x, emittance.y, emittance.z, 1.f}};
	sphere.surfaceCharacteristic = {{surfaceCharacteristic.x, surfaceCharacteristic.y, surfaceCharacteristic.z}};

	float totalEmittance;

	if ((totalEmittance = glm::length(emittance)) > FLT_EPSILON)
	{
		LightSphere lightSphere{};

		lightSphere.radiance = totalEmittance;
		lightSphere.sphereId = spheres.size();

		lightSpheres.push_back(lightSphere);

		sceneInfo.lightSphereCount++;
		sceneInfo.totalRadiance += totalEmittance;
	}

	sceneInfo.sphereCount++;

	spheres.push_back(sphere);
}

Camera Scene::getRenderCamera()
{
	return camera.createRenderCamera();
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
	if (!lightSpheres.empty())
	{
		return &lightSpheres[0];
	}

	return nullptr;
}

size_t Scene::getLightSphereSize()
{
	return sizeof(LightSphere) * lightSpheres.size();
}

SceneInfo *Scene::getSceneInfo()
{
	return &sceneInfo;
}

void Scene::changeResolution(int width, int height)
{
	camera.setResolution(width, height);
	notifyListenerResolutionChanged(width, height);
}

glm::ivec2 Scene::getResolution()
{
	return camera.getResolution();
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
		camera.goForward(delta);
		break;
	case BACKWARD:
		camera.goForward(-delta);
		break;
	case UP:
		camera.changeAltitude(-delta);
		break;
	case DOWN:
		camera.changeAltitude(delta);
		break;
	case RIGHT:
		camera.strafe(-delta);
		break;
	case LEFT:
		camera.strafe(delta);
		break;
	case YAW_RIGHT:
		camera.changeYaw(delta);
		break;
	case YAW_LEFT:
		camera.changeYaw(-delta);
		break;
	case PITCH_UP:
		camera.changePitch(delta);
		break;
	case PITCH_DOWN:
		camera.changePitch(-delta);
		break;
	}

	notifyListenerCameraChanged();
}

void Scene::initialMousePosition(float xPos, float yPos)
{
	camera.setInitialMousePosition(xPos, yPos);
}

void Scene::updateMousePosition(float xPos, float yPos)
{
	camera.handleMouseMovement(xPos, yPos);

	notifyListenerCameraChanged();
}

void Scene::notifyListenerCameraChanged()
{
	if (changeListener)
	{
		changeListener->notify();
	}
}
