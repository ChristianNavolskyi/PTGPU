#ifndef SPHERES_H
#define SPHERES_H

#include "math.cl"
#include "ray.cl"

//####################################  STRUCTS  ##################################################
typedef struct
{
    float radius;
    float dummy1;
    float dummy2;
    float dummy3;
    float3 position;
    float4 color;
    float4 emittance;
    float3 surfaceCharacteristic; // diffuse, specular, transmissive distribution
} Sphere;

typedef struct
{
	int sphereId;
	int radiance;
} LightSphere;


//####################################  DECLARATIONS  #############################################
bool intersectSphere(Sphere *sphere, Ray *ray, float *t1, float *t2);


//####################################  IMPLEMENTATIONS  ##########################################
bool intersectSphere(Sphere *sphere, Ray *ray, float *t1, float *t2)
{
    float3 center = sphere->position;
    float radius = sphere->radius;

    float3 direction = ray->direction;
    float3 origin = ray->origin;
    float3 centerToOrigin = origin - center;

    float a = dot(direction, direction);
    float b = 2 * dot(direction, centerToOrigin);
    float c = dot(centerToOrigin, centerToOrigin) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < EPSILON)
    {
        *t1 = INFINITY;
        *t2 = INFINITY;

        return false;
    }
    else if (discriminant - EPSILON == 0.f)
    {
        *t1 = (-b + sqrt(discriminant)) / (2 * a);
        *t2 = INFINITY;

        return true;
    }
    else if (discriminant > EPSILON)
    {
        *t1 = (-b - sqrt(discriminant)) / (2 * a);
        *t2 = (-b + sqrt(discriminant)) / (2 * a);

        return true;
    }

    return false;
}


#endif