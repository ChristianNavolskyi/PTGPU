
typedef struct Ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct Sphere {
	float radius;
	float dummy1;
	float dummy2;
	float dummy3;
	float3 position;
	float4 color;
	float4 emittance;
	int surfaceCharacteristic;
} Sphere;

typedef struct Intersection {
    float3 position;
    float3 normal;
    float t;
    Sphere *sphere;
} Intersection;

typedef struct Camera
{
	float3 position;
	float3 view;
	float3 up;
	float2 fov;
	int2 resolution;
} Camera;

#ifndef N_BOUNCES
#define N_BOUNCES 8
#endif

#define DIFFUSE (1 << 0)
#define SPECULAR (1 << 1)

#define DEFAULT 0
#define NORMAL 1
#define DEPTH 2
#define COLOR 3
#define EMITTANCE 4

#define EPSILON 0.00001f
#define PI 3.14159265358979323846

static float noise3D(float x, float y, float z);
float3 sampleCosHemisphere(float exp, float rand1, float rand2);
float3 transformIntoWorldSpace(float3 normal, float3 direction);
Ray getCameraRay(__constant Camera *camera, int x, int y, const int iteration, const float seed);
bool intersectSphere(Sphere *sphere, Ray *ray, float *t1, float *t2);
bool findIntersection(Ray *cameraRay, Intersection *intersection, __constant Sphere *spheres, int sphereCount);


static float random(float x, float y, float z) {
    float ptr = 0.0f;
    return fract(sin(x*112.9898f + y*179.233f + z*237.212f) * 43758.5453f, &ptr);
}

float3 sampleCosHemisphere(float exp, float rand1, float rand2) {
    float phi = 2 * M_PI * rand1;
    float theta = acos(pow(1 - rand2, 1 / (exp + 1)));

    return (float3) (sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

float3 transformIntoWorldSpace(float3 normal, float3 direction) {
    float3 e = normalize((float3) (0.f, -normal.z, normal.y));
    float3 f = normalize(cross(normal, e));

    return (float3) (dot(e, direction), dot(f, direction), dot(normal, direction));
}

Ray getCameraRay(__constant Camera *camera, int x, int y, const int iteration, const float seed) {
    float3 camOrigin = camera->position;
    float3 up = camera->up;
    float3 viewDirection = camera->view;
    int2 resolution = camera->resolution;

    float3 wVector = normalize(-viewDirection);
    float3 uVector = normalize(cross(up, wVector));
    float3 vVector = normalize(cross(wVector, uVector));

    float horizontalFactor = tan(camera->fov.x * 0.5f * (PI / 180));
    float verticalFactor = tan(camera->fov.y * -0.5f * (PI / 180));

    float3 middle = camOrigin + viewDirection;
    float3 horizontal = uVector * horizontalFactor;
    float3 vertical = vVector * verticalFactor;

    int xPixel = x;
    int yPixel = y;//resolution.y - y - 1;

    // pixel offset for anti-aliasing
    float randX = random((x + 7.5f) * 1000.f / 3.f, (float) iteration, seed);
    float randY = random((y + 2.5f) * 1000.f / 3.f, (float) iteration, seed);

    float sx = (float) (xPixel + randX - 0.5f) / ((float) resolution.x - 1.f);
    float sy = (float) (yPixel + randY - 0.5f) / ((float) resolution.y - 1.f);

    float3 pointOnImagePlane = middle + horizontal * (2.f * sx - 1.f) + vertical * (2.f * sy - 1.f);

    Ray ray;
    ray.origin = camOrigin;
    ray.direction = normalize(pointOnImagePlane - camOrigin);

    return ray;
}

bool intersectSphere(Sphere *sphere, Ray *ray, float *t1, float *t2) {
    float3 center = sphere->position;
    float radius = sphere->radius;

    float3 direction = ray->direction;
    float3 origin = ray->origin;
    float3 centerToOrigin = origin - center;

    float a = dot(direction, direction);
    float b = 2 * dot(direction, centerToOrigin);
    float c = dot(centerToOrigin, centerToOrigin) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < EPSILON) {
        return false;
    } else if (discriminant - EPSILON == 0.f) {
        float t = (-b + sqrt(discriminant)) / (2 * a);
        *t1 = t;
        *t2 = INFINITY;
        return true;
    } else if (discriminant > EPSILON) {
        *t1 = (-b - sqrt(discriminant)) / (2 * a);
        *t2 = (-b + sqrt(discriminant)) / (2 * a);
        return true;
    }

    return false;
}

bool findIntersection(Ray *cameraRay, Intersection *intersection, __constant Sphere *spheres, int sphereCount) {
    intersection->t = INFINITY;

    for (int i = 0; i < sphereCount; i++) {
        Sphere sphere = spheres[i];
        float t1;
        float t2;

        if (intersectSphere(&sphere, cameraRay, &t1, &t2)) {
            if (t1 > 0.f && t1 < intersection->t) {
                intersection->t = t1;
            }
            if (t2 > 0.f && t2 < intersection->t) {
                intersection->t = t2;
            }
            intersection->sphere = &sphere;
        }
    }

    if (intersection->t < INFINITY) {
        intersection->position = cameraRay->origin + intersection->t * cameraRay->direction;
        intersection->normal = normalize(intersection->position - intersection->sphere->position);

        return true;
    }

    return false;
}

__kernel void render(__global float4 *image, __constant Sphere *spheres, const int sphereCount, __constant Camera *camera, const int iteration, const float seed, const int options) {
    int gx = get_global_id(0);
    int gy = get_global_id(1);
    int width = camera->resolution.x;
    int height = camera->resolution.y;

    int position = gy * width + gx;

    Ray ray = getCameraRay(camera, gx, gy, iteration, seed);

    float4 accumulatedColor = (float4) (0.f, 0.f, 0.f, 0.f);
    float4 mask = (float4) (1.f, 1.f, 1.f, 1.f);

    for (int i = 0; i < N_BOUNCES; i++) {
        Intersection intersection;

        if (findIntersection(&ray, &intersection, spheres, sphereCount)) {
            if (options != DEFAULT) {
                if (options == NORMAL) {
                    accumulatedColor.rgb = (intersection.normal + (float3) (1.f, 1.f, 1.f)) / 2.f;
                } else if (options == DEPTH) {
                    accumulatedColor = intersection.t / 10.f;
                } else if (options == COLOR) {
                    accumulatedColor = intersection.sphere->color;
                } else if (options == EMITTANCE) {
                    accumulatedColor = intersection.sphere->emittance;
                }

                break;
            }

            float3 newDirection;

            if (intersection.sphere->surfaceCharacteristic == DIFFUSE) {
                float rand1 = random((float) gx, (float) gy, iteration * seed);
                float rand2 = random((float) gx, (float) gy, iteration + seed);

                float3 w = intersection.normal;
                float3 u = normalize((float3) (0.f, -w.z, w.y));
                float3 v = normalize(cross(w, u));

                float3 directionInHemisphere = sampleCosHemisphere(0.f, rand1, rand2);

                newDirection = normalize(u * directionInHemisphere.x + v * directionInHemisphere.y + w * directionInHemisphere.z);
            } else if (intersection.sphere->surfaceCharacteristic == SPECULAR) {
                newDirection = ray.direction + 2 * dot(-ray.direction, intersection.normal);
            }

            ray.origin = intersection.position + intersection.normal * EPSILON;
            ray.direction = newDirection;

            accumulatedColor += mask * intersection.sphere->emittance;

            mask *= intersection.sphere->color;

            mask *= dot(ray.direction, intersection.normal);
        } else {
            accumulatedColor += mask * (float4) (0.15f, 0.15f, 0.25f, 1.f);
            break;
        }
    }

    if (gx < width && gy < height) {
        image[position] = (image[position] * iteration + accumulatedColor) / (iteration + 1.f);
//        if (iteration == 0) {
//            image[position] = accumulatedColor;
//        } else {
//            image[position] += accumulatedColor;
//        }
    }
}