
typedef struct Ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct Sphere {
	float radius;
	float3 position;
	float3 color;
	float3 emission;
} Sphere;

typedef struct Intersection {
    float3 position;
    float3 normal;
    float t;
    Sphere *sphere;
} Intersection;

typedef struct Camera{
	float3 position;
	float3 view;
	float3 up;
	float2 resolution;
	float2 fov;
	float focalDistance;
} Camera;

#ifndef N_BOUNCES
#define N_BOUNCES 1
#endif

#ifndef CAMERA_DISTANCE
#define CAMERA_DISTANCE 1.f
#endif

#define EPSILON 0.00001f

static float noise3D(float x, float y, float z);
float3 sampleCosHemisphere(float exp, float rand1, float rand2);
float3 transformIntoWorldSpace(float3 normal, float3 direction);
void setPixelColor3f(__global float* image, int position, float3 color);
Ray getCameraRay(int x, int y, int width, int height);
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

void setPixelColor3f(__global float* image, int position, float3 color) {
    image[position] = color.r;
    image[position + 1] = color.g;
    image[position + 2] = color.b;
}

Ray getCameraRay(int x, int y, int width, int height) {
    float3 camOrigin = (float3) (0.f, 0.f, 0.f);
    float3 up = (float3) (0.f, 0.f, 1.f);
    float3 viewDirection = (float3) (0.f, 1.f, 0.f);

    float3 wVector = normalize(-viewDirection);
    float3 uVector = normalize(cross(up, wVector));
    float3 vVector = normalize(cross(wVector, uVector));

    float l = -1.f;
    float r = 2.f;
    float t = 1.f;
    float b = -t;

    // TODO replace .5f with random number in [0, 1) for AA
    float u = l + (r - l) * (x + random(x, y, 1.f)) / (float) width;
    float v = t + (b - t) * (y + random(x, y, 2.f)) / (float) height;

    float d = CAMERA_DISTANCE;
    float3 s = u * uVector + v * vVector - d * wVector;

    Ray ray;
    ray.origin = camOrigin;
    ray.direction = normalize(s);

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

__kernel void render(__global float *image, __constant Sphere *spheres, const int sphereCount, const int width, const int height, const int iteration) {
    int gx = get_global_id(0);
    int gy = get_global_id(1);
    int pixelPosition = gy * width * 3 + gx * 3;

    Ray ray = getCameraRay(gx, gy, width, height);

    float3 accumulatedColor = (float3) (0.f, 0.f, 0.f);
    float3 mask = (float3) (1.f, 1.f, 1.f);

    for (int i = 0; i < N_BOUNCES; i++) {
        Intersection intersection;

        if (findIntersection(&ray, &intersection, spheres, sphereCount)) {
            accumulatedColor += mask * (float3) (1.f, 0.4f, 0.7f);
            break;

            float rand1 = random((float) gx, (float) gy, (float) iteration);
            float rand2 = random((float) gx, (float) gy, (float) iteration * iteration);
            float rand2s = sqrt(rand2);

            float3 w = intersection.normal;
            float3 u = normalize((float3) (0.f, -w.z, w.y));
            float3 v = normalize(cross(w, u));

            float3 newdir = normalize(u * cos(rand1)*rand2s + v*sin(rand1)*rand2s + w*sqrt(1.0f - rand2));

            float3 sampleOnHemisphere = transformIntoWorldSpace(intersection.normal, sampleCosHemisphere(0.f, rand1, rand2));

            ray.origin = intersection.position + intersection.normal * EPSILON;
            ray.direction = newdir;

            accumulatedColor += mask * intersection.sphere->emission;

            mask *= intersection.sphere->color;

            mask *= dot(ray.direction, intersection.normal);
        } else {
            accumulatedColor += mask * (float3) (0.3f, 0.3f, 0.7f);
            break;
        }
    }

    setPixelColor3f(image, pixelPosition, accumulatedColor);
}