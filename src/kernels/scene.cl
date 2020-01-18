#ifndef SCENE_H
#define SCENE_H

#include "ray.cl"
#include "spheres.cl"
#include "triangles.cl"

//####################################  STRUCTS  ##################################################
typedef struct
{
	int sphereCount;
	int lightSphereCount;
	int triangleCount;
	int lightTriangleCount;
	int totalRadiance;
	float3 backgroundColor;
	__constant Sphere *spheres;
	__constant LightSphere *lightSpheres;
	__constant Triangle *triangles;
	__constant LightTriangle *lightTriangles;
} Scene;

// Object Type Definition
#define SPHERE 1
#define TRIANGLE 2

typedef struct
{
    float3 position;
    float3 normal;
    float t;
    int objectType;
    union {
        struct {
            int sphereId;
            __constant Sphere *sphere;
        };
        struct {
            int triangleId;
            __constant Triangle *triangle;
        };
    };
} Intersection;


//####################################  DECLARATIONS  #############################################
bool findIntersection(Ray *ray, Intersection *intersection, Scene *scene);


//####################################  IMPLEMENTATIONS  ##########################################
bool findIntersection(Ray *ray, Intersection *intersection, Scene *scene)
{
    intersection->t = INFINITY;

    for (int i = 0; i < scene->sphereCount; i++)
    {
        Sphere sphere = scene->spheres[i];
        float t1;
        float t2;

        if (intersectSphere(&sphere, ray, &t1, &t2))
        {
            if (t1 > 0.f && t1 < intersection->t)
            {
                intersection->t = t1;

                intersection->sphereId = i;
                intersection->sphere = &scene->spheres[i];
            }
            if (t2 > 0.f && t2 < intersection->t)
            {
                intersection->t = t2;

                intersection->sphereId = i;
                intersection->sphere = &scene->spheres[i];
            }
        }
    }

    if (intersection->t < INFINITY)
    {
        intersection->position = ray->origin + intersection->t * ray->direction;
        intersection->normal = normalize(intersection->position - intersection->sphere->position);

        return true;
    }

    return false;
}



#endif