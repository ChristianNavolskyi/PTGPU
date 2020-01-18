#ifndef RAY_H
#define RAY_H

//####################################  STRUCTS  ##################################################
typedef struct
{
    float3 origin;
    float3 direction;
} Ray;


//####################################  DECLARATIONS  #############################################
float3 reflect(Ray *ray, float3 normal);


//####################################  IMPLEMENTATIONS  ##########################################
float3 reflect(Ray *ray, float3 normal)
{
    return ray->direction + 2.f * normal * dot(normal, -ray->direction);
}

#endif