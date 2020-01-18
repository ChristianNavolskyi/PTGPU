#ifndef CAMERA_H
#define CAMERA_H

#include "ray.cl"

//####################################  STRUCTS  ##################################################
typedef struct
{
    float3 position;
    float3 view;
    float3 up;
    float2 fov;
    int2 resolution;
} Camera;


//####################################  DECLARATIONS  #############################################
Ray getCameraRay(__constant Camera *camera, int x, int y, const int iteration, const float seed);


//####################################  IMPLEMENTATIONS  ##########################################
Ray getCameraRay(__constant Camera *camera, int x, int y, const int iteration, const float seed)
{
    float3 camOrigin = camera->position;
    float3 up = camera->up;
    float3 viewDirection = camera->view;
    int2 resolution = camera->resolution;

    float3 wVector = normalize(-viewDirection);
    float3 uVector = normalize(cross(up, wVector));
    float3 vVector = normalize(cross(wVector, uVector));

    // TODO fov -> aspect ratio
    float horizontalFactor = tan(camera->fov.x * 0.5f * (M_PI_F / 180));
    float verticalFactor = tan(camera->fov.y * -0.5f * (M_PI_F / 180));

    float3 middle = camOrigin + viewDirection;
    float3 horizontal = uVector * horizontalFactor;
    float3 vertical = vVector * verticalFactor;

    int xPixel = x;
    int yPixel = resolution.y - y - 1;

    // pixel offset for anti-aliasing
    float randX = random((x + 7.5f) * 1000.f / 3.f, (float)iteration, seed);
    float randY = random((y + 2.5f) * 1000.f / 3.f, (float)iteration, seed);

    float sx = (float)(xPixel + randX - 0.5f) / ((float)resolution.x - 1.f);
    float sy = (float)(yPixel + randY - 0.5f) / ((float)resolution.y - 1.f);

    float3 pointOnImagePlane = middle + horizontal * (2.f * sx - 1.f) + vertical * (2.f * sy - 1.f);

    Ray ray;
    ray.origin = camOrigin;
    ray.direction = normalize(pointOnImagePlane - camOrigin);

    return ray;
}


#endif
