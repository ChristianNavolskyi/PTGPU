
typedef struct
{
    float3 origin;
    float3 direction;
} Ray;

typedef struct
{
    float radius;
    float dummy1;
    float dummy2;
    float dummy3;
    float3 position;
    float4 color;
    float4 emittance;
    float3 surfaceCharacteristic;
} Sphere;

typedef struct {
    Sphere *lightSpheres;
    int *lightSphereIds;
    float *lightSphereRadiance;
    float totalRadiance;
    int lightSphereCount;
} LightSphereHolder;

typedef struct
{
    float3 position;
    float3 normal;
    float t;
    int sphereId;
    __constant Sphere *sphere;
} Intersection;

typedef struct
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
#define SPHERE_ID 5
#define RANDOM 6

#define EPSILON 0.00001f

float3 reflect(Ray *ray, float3 normal);

int selectLightSpheres(__constant Sphere *spheres, Sphere *lightSpheres, int *lightSphereIds, float *lightSphereRadiance, int sphereCount, float *totalRadiance);
float4 showDebugVision(Intersection *intersection, int options, float rand0, float rand1, float rand2);
float4 nextEventEstimation(__constant Sphere *spheres, int sphereCount, Intersection *intersection, LightSphereHolder *lightSphereHolder, float rand0, float rand1, float rand2);

static float random(float x, float y, float z);
float3 sampleCosHemisphere(float exp, float rand1, float rand2);
float sampleCosHemispherePDF(float exp, float cosTheta);
float3 transformIntoWorldSpace(float3 w, float3 direction);
float3 transformIntoTangentSpace(float3 w, float3 direction);
Ray getCameraRay(__constant Camera *camera, int x, int y, const int iteration, const float seed);
bool intersectSphere(Sphere *sphere, Ray *ray, float *t1, float *t2);
bool findIntersection(Ray *ray, Intersection *intersection, __constant Sphere *spheres, int sphereCount);

float3 reflect(Ray *ray, float3 normal)
{
    return ray->direction + 2.f * normal * dot(normal, -ray->direction);
}

int selectLightSpheres(__constant Sphere *spheres, Sphere *lightSpheres, int *lightSphereIds, float *lightSphereRadiance, int sphereCount, float *totalRadiance)
{
    int lightSphereCount = 0;
    *totalRadiance = 0;

    for (int i = 0; i < sphereCount; i++)
    {
        Sphere currentSphere = spheres[i];
        float4 emittance = currentSphere.emittance;

        if (length(emittance.xyz) > EPSILON)
        { // emittance is not zero
            lightSpheres[lightSphereCount] = spheres[i];
            lightSphereRadiance[lightSphereCount] = emittance.x + emittance.y + emittance.z;
            lightSphereIds[lightSphereCount] = i;

            *totalRadiance += lightSphereRadiance[lightSphereCount];

            lightSphereCount++;
        }
    }

    return lightSphereCount;
}

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

float4 nextEventEstimation(__constant Sphere *spheres, int sphereCount, Intersection *intersection, LightSphereHolder *lightSphereHolder, float rand0, float rand1, float rand2)
{
    float4 directLight = 1.f;

    float lightSelector = rand0;
    Sphere *selectedLightSource;
    int selectedLightSourceId;

    for (int i = 0; i < lightSphereHolder->lightSphereCount; i++)
    {
        float radianceContribution = lightSphereHolder->lightSphereRadiance[i] / lightSphereHolder->totalRadiance;
        if (lightSelector < radianceContribution)
        {
            selectedLightSource = &lightSphereHolder->lightSpheres[i];
            directLight /= radianceContribution;
            selectedLightSourceId = lightSphereHolder->lightSphereIds[i];
        }
        else
        {
            lightSelector -= lightSphereHolder->lightSphereRadiance[i];
        }
    }

    float3 directionToLight = normalize(selectedLightSource->position - intersection->position);

    Ray lightRay;

    if (selectedLightSource->radius < EPSILON)
    { // point light source
        lightRay.direction = normalize(selectedLightSource->position - intersection->position);

        directLight /= (4 * M_PI_F); // pdf
        directLight /= pow(length(selectedLightSource->position - intersection->position), 2.f); // distance falloff
        directLight *= dot(intersection->normal, directionToLight); // cos theta
    }
    else
    {
        float3 hemisphereSample = sampleCosHemisphere(0.f, rand1, rand2);
        float3 pointOnLightSphere = selectedLightSource->position + transformIntoTangentSpace(-directionToLight, hemisphereSample);
        lightRay.direction = normalize(pointOnLightSphere - intersection->position);

        float3 normalAtLightPoint = normalize(pointOnLightSphere - selectedLightSource->position);

        directLight /= sampleCosHemispherePDF(0, hemisphereSample.z); // pdf
        directLight /= pow(length(intersection->position - pointOnLightSphere), 2.f); // distance falloff
        directLight *= dot(intersection->normal, lightRay.direction) * dot(normalAtLightPoint, -lightRay.direction); // cos theta i and j
    }

    lightRay.origin = intersection->position + intersection->normal * EPSILON;

    Intersection lightIntersection;

    if (findIntersection(&lightRay, &lightIntersection, spheres, sphereCount) && lightIntersection.sphereId == selectedLightSourceId)
    { // first intersection is with selected light source
        directLight *= selectedLightSource->emittance;
    }
    else
    {
        directLight = 0.f;
    }

    return directLight;
}

static float random(float x, float y, float z)
{
    float ptr = 0.0f;
    return fract(sin(x * 112.9898f + y * 179.233f + z * 237.212f) * 43758.5453f, &ptr);
}

float3 sampleCosHemisphere(float exp, float rand1, float rand2)
{
    float phi = 2 * M_PI_F * rand1;
    float theta = acos(pow(1 - rand2, 1 / (exp + 1)));

    return (float3)(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

float sampleCosHemispherePDF(float exp, float cosTheta)
{
    return (exp + 1) / (2 * M_PI_F) * pow(cosTheta, exp);
}

float3 transformIntoWorldSpace(float3 w, float3 direction)
{
    float3 u = normalize((float3)(0.f, -w.z, w.y));
    float3 v = normalize(cross(w, u));

    return (float3)(dot(u, direction), dot(v, direction), dot(w, direction));
}

float3 transformIntoTangentSpace(float3 w, float3 direction)
{
    float3 u = normalize((float3)(0.f, -w.z, w.y));
    float3 v = normalize(cross(w, u));

    float3 uTransposed = (float3)(u.x, v.x, w.x);
    float3 vTransposed = (float3)(u.y, v.y, w.y);
    float3 wTransposed = (float3)(u.z, v.z, w.z);

    return (float3)(dot(uTransposed, direction), dot(vTransposed, direction), dot(wTransposed, direction));
}

Ray getCameraRay(__constant Camera *camera, int x, int y, const int iteration, const float seed)
{
    float3 camOrigin = camera->position;
    float3 up = camera->up;
    float3 viewDirection = camera->view;
    int2 resolution = camera->resolution;

    float3 wVector = normalize(-viewDirection);
    float3 uVector = normalize(cross(up, wVector));
    float3 vVector = normalize(cross(wVector, uVector));

    float horizontalFactor = tan(camera->fov.x * 0.5f * (M_PI_F / 180));
    float verticalFactor = tan(camera->fov.y * -0.5f * (M_PI_F / 180));

    float3 middle = camOrigin + viewDirection;
    float3 horizontal = uVector * horizontalFactor;
    float3 vertical = vVector * verticalFactor;

    int xPixel = x;
    int yPixel = y; //resolution.y - y - 1;

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

bool findIntersection(Ray *ray, Intersection *intersection, __constant Sphere *spheres, int sphereCount)
{
    intersection->t = INFINITY;

    for (int i = 0; i < sphereCount; i++)
    {
        Sphere sphere = spheres[i];
        float t1;
        float t2;

        if (intersectSphere(&sphere, ray, &t1, &t2))
        {
            if (t1 > 0.f && t1 < intersection->t)
            {
                intersection->t = t1;

                intersection->sphereId = i;
                intersection->sphere = &spheres[i];
            }
            if (t2 > 0.f && t2 < intersection->t)
            {
                intersection->t = t2;

                intersection->sphereId = i;
                intersection->sphere = &spheres[i];
            }
        }
    }

    if (intersection->t < INFINITY)
    {
        intersection->position = ray->origin + intersection->t * ray->direction;
        intersection->normal = normalize(intersection->position - intersection->sphere->position);
        //        float3 normal = normalize(intersection->position - intersection->sphere->position);
        //        float3 normal_facing = dot(normal, ray->direction) < 0.0f ? normal : normal * (-1.0f);
        //
        //        intersection->normal = normal_facing;

        return true;
    }

    return false;
}

__kernel void render(__global float4 *image, __constant Sphere *spheres, const int sphereCount, __constant Camera *camera, const int iteration, const float seed, const int options)
{
    int gx = get_global_id(0);
    int gy = get_global_id(1);
    int width = camera->resolution.x;
    int height = camera->resolution.y;

    int position = gy * width + gx;

    Sphere lightSpheres[sphereCount];
    int lightSphereIds[sphereCount];
    float lightSphereRadiance[sphereCount];
    float totalRadiance = 0;
    int lightSphereCount = selectLightSpheres(spheres, lightSpheres, lightSphereIds, lightSphereRadiance, sphereCount, &totalRadiance);

    LightSphereHolder lightSphereHolder;
    lightSphereHolder.lightSpheres = lightSpheres;
    lightSphereHolder.lightSphereIds = lightSphereIds;
    lightSphereHolder.lightSphereRadiance = lightSphereRadiance;
    lightSphereHolder.totalRadiance = totalRadiance;
    lightSphereHolder.lightSphereCount = lightSphereCount;

    Ray ray = getCameraRay(camera, gx, gy, iteration, seed);

    float4 L = (float4)(0.f, 0.f, 0.f, 0.f);
    float4 brdfCosFactor = (float4) (1.f);

    for (int i = 0; i < N_BOUNCES; i++)
    {
        Intersection intersection;

        if (findIntersection(&ray, &intersection, spheres, sphereCount))
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
//
            brdfCosFactor *= intersection.sphere->color;
//
            float4 directLight = nextEventEstimation(spheres, sphereCount, &intersection, &lightSphereHolder, rand0, rand1, rand2);
            L += directLight * brdfCosFactor;

            float3 newDirection;
            float rand3 = random((gx + iteration) / (float) width, (gy + iteration) / (float) height, seed); // select which surface characteristic is used for the next ray direction
            float3 selectedSphereCharacteristic = intersection.sphere->surfaceCharacteristic;

            if (rand3 < selectedSphereCharacteristic.x) { // diffuse reflection
                float rand4 = random((gx + iteration) / (float) width, gy / (float) height, seed + iteration);
                float rand5 = random(gx / (float) width, (gy + iteration) / (float) height, seed + iteration);
                float3 directionInHemisphere = sampleCosHemisphere(0.f, rand4, rand5);
                float directionPDF = sampleCosHemispherePDF(0.f, directionInHemisphere.z); // TODO check where to divide

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
//                        L += (float4)((ray.direction + 1.f) / 2.f, 1.f);
            L += (float4)(0.7f) * brdfCosFactor;
            break;
        }
    }

    image[position] = (image[position] * iteration + L) / (iteration + 1.f);
}