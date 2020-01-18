#ifndef PATHTRACER_H
#define PATHTRACER_H

#include "scene.cl"
#include "camera.cl"
#include "ray.cl"
#include "math.cl"


//####################################  DEFINES  ##################################################
#ifndef N_BOUNCES
#define N_BOUNCES 5
#endif

#define DEFAULT 0
#define NORMAL 1
#define DEPTH 2
#define COLOR 3
#define EMITTANCE 4
#define SPHERE_ID 5
#define RANDOM 6


//####################################  DECLARATIONS  #############################################
float4 showDebugVision(Intersection *intersection, int options, float rand0, float rand1, float rand2);

float4 nextEventEstimation(Scene *scene, Intersection *intersection, float rand0, float rand1, float rand2);


//####################################  IMPLEMENTATIONS  ##########################################
float4 showDebugVision(Intersection *intersection, int options, float rand0, float rand1, float rand2)
{
    if (options == NORMAL)
    {
        return (float4)((intersection->normal + (float3)(1.f, 1.f, 1.f)) / 2.f, 1.f);
    }
    else if (options == DEPTH)
    {
        return (float4)intersection->t / 5.f;
    }
    else if (options == COLOR)
    {
        return intersection->sphere->color;
    }
    else if (options == EMITTANCE)
    {
        return intersection->sphere->emittance;
    }
    else if (options == SPHERE_ID)
    {
        return (float4)intersection->sphereId / 10.f;
    }
    else if (options == RANDOM)
    {
        return (float4) (rand0, rand1, rand2, 1.f);
    }

    return (float4)0.f;
}

float4 nextEventEstimation(Scene *scene, Intersection *intersection, float rand0, float rand1, float rand2)
{
    float4 directLight = (float4) 1.f;

    float lightSelector = rand0;
    __constant Sphere *selectedLightSource;
    int selectedLightSourceId;

    float radianceContribution = 0.f;

    for (int i = 0; i < scene->lightSphereCount; i++)
    {
        __constant LightSphere *currentLightSphere = &scene->lightSpheres[i];
        float currentRadianceContribution = currentLightSphere->radiance / scene->totalRadiance;
        radianceContribution += currentRadianceContribution;

        if (lightSelector < radianceContribution)
        {
            selectedLightSourceId = currentLightSphere->sphereId;
            selectedLightSource = &scene->spheres[selectedLightSourceId];
            directLight /= currentRadianceContribution;
            break;
        }
    }

    float3 directionToLight = normalize(selectedLightSource->position - intersection->position);
    float distanceToPoint = INFINITY;

    Ray lightRay;

    if (selectedLightSource->radius < EPSILON)
    { // point light source
        float3 vectorToPoint = selectedLightSource->position - intersection->position;

        lightRay.direction = normalize(vectorToPoint);
        distanceToPoint = length(vectorToPoint);

        directLight /= (4 * M_PI_F); // radiance contribution of point into one direction
        directLight /= pow(distanceToPoint, 2.f); // distance falloff
        directLight *= dot(intersection->normal, directionToLight); // cos theta
    }
    else
    {
        float3 hemisphereSample = sampleCosHemisphere(0.f, rand1, rand2);
        float3 pointOnLightSphere = selectedLightSource->position + transformIntoTangentSpace(-directionToLight, hemisphereSample) * selectedLightSource->radius;

        lightRay.direction = normalize(pointOnLightSphere - intersection->position);

        float3 normalAtLightPoint = normalize(pointOnLightSphere - selectedLightSource->position);

        directLight /= 2 * M_PI_F * pow(selectedLightSource->radius, 2.f); // pdf - surface area half
        directLight /= pow(length(intersection->position - pointOnLightSphere), 2.f); // distance falloff
        directLight *= dot(intersection->normal, lightRay.direction) * dot(normalAtLightPoint, -lightRay.direction); // cos theta i and j
    }

    lightRay.origin = intersection->position + intersection->normal * EPSILON;

    Intersection lightIntersection;

    if ((findIntersection(&lightRay, &lightIntersection, scene) && lightIntersection.sphereId == selectedLightSourceId) || distanceToPoint < lightIntersection.t)
    { // first intersection is with selected light source
        directLight *= selectedLightSource->emittance;
    }
    else
    {
        directLight = 0.f;
    }

    return directLight;
}

__kernel void render(__global float4 *image, __constant Sphere *spheres, __constant LightSphere *lightSpheres, __constant Triangle *triangles, __constant LightTriangle *lightTriangles, __constant Scene *sceneInfo, __constant Camera *camera, const int iteration, const float seed, const int options)
{
    int gx = get_global_id(0);
    int gy = get_global_id(1);
    int width = camera->resolution.x;
    int height = camera->resolution.y;

    Scene scene = *sceneInfo;

    int position = gy * width + gx;

    Ray ray = getCameraRay(camera, gx, gy, iteration, seed);

    float4 L = (float4)(0.f, 0.f, 0.f, 0.f);
    float4 brdfCosFactor = (float4) (1.f);

    for (int i = 0; i < N_BOUNCES; i++)
    {
        Intersection intersection;

        if (findIntersection(&ray, &intersection, &scene))
        {
            float rand0 = random((gx + iteration) / (float)width, gy / (float)height, seed);
            float rand1 = random(gx / (float)width, (gy + iteration) / (float)height, seed);
            float rand2 = random(gx / (float)width, gy / (float)height, seed + iteration);

            if (options != DEFAULT)
            {
                L = showDebugVision(&intersection, options, rand0, rand1, rand2);

                break;
            }

            float4 emittedLight = intersection.sphere->emittance;
            L += emittedLight * brdfCosFactor;

            brdfCosFactor *= intersection.sphere->color;

            float4 directLight = nextEventEstimation(&scene, &intersection, rand0, rand1, rand2);
            L += directLight * brdfCosFactor;

            float3 newDirection;
            float rand3 = random((gx + iteration) / (float) width, (gy + iteration) / (float) height, seed); // select which surface characteristic is used for the next ray direction
            float3 selectedSphereCharacteristic = intersection.sphere->surfaceCharacteristic;

            if (rand3 < selectedSphereCharacteristic.x) { // diffuse reflection
                float rand4 = random((gx + seed) / (float) width, (gy + iteration) / (float) height, seed);
                float rand5 = random((gx + iteration) / (float) width, (gy + seed) / (float) height, seed);
                float3 directionInHemisphere = sampleCosHemisphere(0.f, rand4, rand5);
//                float directionPDF = sampleCosHemispherePDF(0.f, directionInHemisphere.z); // TODO check where to divide

                newDirection = transformIntoTangentSpace(intersection.normal, directionInHemisphere);
            } else if (rand3 < selectedSphereCharacteristic.x + selectedSphereCharacteristic.y) { // specular reflection
                newDirection = reflect(&ray, intersection.normal);
            } else { // transmission
            // Fresnel / Snellsches Brechungsgesetz

            }

            ray.origin = intersection.position + intersection.normal * EPSILON;
            ray.direction = newDirection;

            brdfCosFactor *= dot(intersection.normal, ray.direction);
        }
        else
        {
            L += (float4)(sceneInfo->backgroundColor, 1.f) * brdfCosFactor;
            break;
        }
    }

    image[position] = (image[position] * iteration + L) / (iteration + 1.f);
}

#endif