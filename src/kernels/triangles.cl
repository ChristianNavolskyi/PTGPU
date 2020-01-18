#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "ray.cl"

//####################################  STRUCTS  ##################################################
typedef struct Triangle
{
	float3 p1, p2, p3;
	float3 color;
	float3 emittance;
	float3 surfaceCharacteristic;
} Triangle;

typedef struct LightTriangle
{
	int triangleId;
	int radiance;
} LightTriangle;


//####################################  DECLARATIONS  #############################################
bool intersectTriangle(Triangle *, Ray *ray, float *t1);


//####################################  IMPLEMENTATIONS  ##########################################



#endif