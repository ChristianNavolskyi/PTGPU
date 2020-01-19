//************************************** MATH *****************************************************
//####################################  DEFINES  ##############################################
#define EPSILON 0.00001f

//####################################  DECLARATIONS  #############################################
void createOrthonormalAxes(float3 z, float3 *x, float3 *y);
float random(float x, float y, float z);
float3 sampleCosHemisphere(float exp, float rand1, float rand2);
float sampleCosHemispherePDF(float exp, float cosTheta);
float3 transformIntoWorldSpace(float3 w, float3 direction);
float3 transformIntoTangentSpace(float3 w, float3 direction);

//####################################  IMPLEMENTATIONS  ##########################################
float random(float x, float y, float z)
{
    float ptr = 0.0f;
    return fract(sin(x * 112.9898f + y * 179.233f + z * 237.212f) * 43758.5453f, &ptr);
}

float3 sampleCosHemisphere(float exp, float rand1, float rand2)
{
    float phi = 2 * M_PI_F * rand1;
    float theta = acos(pow(1 - rand2, 1 / (exp + 1)));

    return normalize((float3)(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)));
}

float sampleCosHemispherePDF(float exp, float cosTheta)
{
    return (exp + 1) / (2 * M_PI_F) * pow(cosTheta, exp);
}

void createOrthonormalAxes(float3 z, float3 *x, float3 *y)
{
    float3 helpAxis = fabs(z.x) > 0.1f ? (float3)(0.f, 1.f, 0.f) : (float3)(1.f, 0.f, 0.f);

    *x = normalize(cross(helpAxis, z));
    *y = normalize(cross(z, *x));
}

float3 transformIntoWorldSpace(float3 w, float3 direction)
{
    float3 u;
    float3 v;

    createOrthonormalAxes(w, &u, &v);

    return (float3)(dot(u, direction), dot(v, direction), dot(w, direction));
}

float3 transformIntoTangentSpace(float3 w, float3 direction)
{
    float3 u;
    float3 v;

    createOrthonormalAxes(w, &u, &v);

    float3 uTransposed = (float3)(u.x, v.x, w.x);
    float3 vTransposed = (float3)(u.y, v.y, w.y);
    float3 wTransposed = (float3)(u.z, v.z, w.z);

    return (float3)(dot(uTransposed, direction), dot(vTransposed, direction), dot(wTransposed, direction));
}
//************************************** MATH *****************************************************

//************************************** RAY ******************************************************
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
//************************************** RAY ******************************************************

//************************************** SPHERES **************************************************
//####################################  STRUCTS  ##################################################
typedef struct
{
    float radius;
    float dummy1;
    float dummy2;
    float dummy3;
    float3 position;
    float3 color;
    float3 emittance;
    float3 surfaceCharacteristic; // diffuse, specular, transmissive distribution
} Sphere;

typedef struct
{
    int sphereId;
    int radiance;
} LightSphere;

//####################################  DECLARATIONS  #############################################
bool intersectSphere(__constant Sphere *sphere, Ray *ray, float *t1);

//####################################  IMPLEMENTATIONS  ##########################################
bool intersectSphere(__constant Sphere *sphere, Ray *ray, float *t1)
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

        return false;
    }
    else if (discriminant - EPSILON == 0.f)
    {
        *t1 = (-b + sqrt(discriminant)) / (2 * a);

        return true;
    }
    else if (discriminant > EPSILON)
    {
        *t1 = (-b - sqrt(discriminant)) / (2 * a);
        *t1 = *t1 < 0.f ? (-b + sqrt(discriminant)) / (2 * a) : *t1;

        return true;
    }

    return false;
}
//************************************** SPHERES **************************************************

//************************************** TRIANGLES ************************************************
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
bool intersectTriangle(__constant Triangle *triangle, Ray *ray, float *t);
float3 getTriangleNormal(__constant Triangle *triangle, Ray *ray);
float getTriangleArea(__constant Triangle *triangle);
float3 sampleUniformPointOnTriangle(__constant Triangle *triangle, float rand0, float rand1);

//####################################  IMPLEMENTATIONS  ##########################################
bool intersectTriangle(__constant Triangle *triangle, Ray *ray, float *t)
{
    float3 e1 = triangle->p2 - triangle->p1;
    float3 e2 = triangle->p3 - triangle->p1;

    float3 s1 = cross(ray->direction, e2);
    float divisor = dot(s1, e1);
    if (divisor == 0.f)
        return false;
    float invDivisor = 1.f / divisor;

    // Compute first barycentric coordinate
    float3 d = ray->origin - triangle->p1;
    float b1 = dot(d, s1) * invDivisor;
    if (b1 < -EPSILON || b1 > 1.f + EPSILON)
        return false;

    // Compute second barycentric coordinate
    float3 s2 = cross(d, e1);
    float b2 = dot(ray->direction, s2) * invDivisor;
    if (b2 < -EPSILON || b1 + b2 > 1.f + EPSILON)
        return false;

    // Compute _t_ to intersection point
    float t1 = dot(e2, s2) * invDivisor;
    if (t1 < -EPSILON)
        return false;

    // Store the closest found intersection so far
    *t = t1;
    return true;
}

float3 getTriangleNormal(__constant Triangle *triangle, Ray *ray)
{
    float3 e1 = triangle->p2 - triangle->p1;
    float3 e2 = triangle->p3 - triangle->p1;

    float3 normal = normalize(cross(e1, e2));
    return dot(ray->direction, normal) < 0 ? normal : -normal;
}

float getTriangleArea(__constant Triangle *triangle)
{
    // Formula from https://math.stackexchange.com/a/128999/630882
    float3 e1 = triangle->p2 - triangle->p1;
    float3 e2 = triangle->p3 - triangle->p1;

    return 0.5f * length(cross(e1, e2));
}

float3 sampleUniformPointOnTriangle(__constant Triangle *triangle, float rand0, float rand1)
{
    // Formula from Photorealistic Image Synthesis
    float s = 1 - sqrt(1 - rand0);
    float t = (1 - s) * rand1;

    return triangle->p1 + s * (triangle->p2 - triangle->p1) + t * (triangle->p3 - triangle->p1);
}
//************************************** TRIANGLES ************************************************

//************************************** SCENE ****************************************************
//####################################  DEFINES  ##################################################
// Object Type Definition
#define SPHERE 1
#define TRIANGLE 2

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

typedef struct
{
    float3 position;
    float3 normal;
    float t;
    int objectType;
    union {
        struct
        {
            int triangleId;
            __constant Triangle *triangle;
        };
        struct
        {
            int sphereId;
            __constant Sphere *sphere;
        };
    };
} Intersection;

//####################################  DECLARATIONS  #############################################
bool findIntersection(Ray *ray, Intersection *intersection, Scene *scene);
float3 getObjectColor(Intersection *intersection);
float3 getObjectEmittance(Intersection *intersection);
float3 getObjectSurfaceCharacteristic(Intersection *intersection);

//####################################  IMPLEMENTATIONS  ##########################################
bool findIntersection(Ray *ray, Intersection *intersection, Scene *scene)
{
    intersection->t = INFINITY;
    float t = INFINITY;

    for (int i = 0; i < scene->sphereCount; i++)
    {
        if (intersectSphere(&scene->spheres[i], ray, &t))
        {
            if (t > 0.f && t < intersection->t)
            {
                intersection->t = t;

                intersection->sphereId = i;
            }
        }
    }

    if (intersection->t < INFINITY)
    {
        intersection->sphere = &scene->spheres[intersection->sphereId];
        intersection->position = ray->origin + intersection->t * ray->direction;
        intersection->normal = normalize(intersection->position - intersection->sphere->position);
        intersection->objectType = SPHERE;
    }

    float sphereT = intersection->t;

    for (int i = 0; i < scene->triangleCount; i++)
    {
        if (intersectTriangle(&scene->triangles[i], ray, &t))
        {
            if (t > 0.f && t < intersection->t)
            {
                intersection->t = t;

                intersection->triangleId = i;
            }
        }
    }

    if (intersection->t < sphereT)
    {
        intersection->triangle = &scene->triangles[intersection->triangleId];
        intersection->position = ray->origin + intersection->t * ray->direction;
        intersection->normal = getTriangleNormal(intersection->triangle, ray);
        intersection->objectType = TRIANGLE;
    }

    if (intersection->t < INFINITY)
    {
        return true;
    }
    else
    {
        return false;
    }
}

float3 getObjectColor(Intersection *intersection)
{
    switch (intersection->objectType)
    {
    case SPHERE:
        return intersection->sphere->color;
    case TRIANGLE:
        return intersection->triangle->color;
    default:
        return (float3)0.f;
    }
}

float3 getObjectEmittance(Intersection *intersection)
{
    switch (intersection->objectType)
    {
    case SPHERE:
        return intersection->sphere->emittance;
    case TRIANGLE:
        return intersection->triangle->emittance;
    default:
        return (float3)0.f;
    }
}

float3 getObjectSurfaceCharacteristic(Intersection *intersection)
{
    switch (intersection->objectType)
    {
    case SPHERE:
        return intersection->sphere->surfaceCharacteristic;
    case TRIANGLE:
        return intersection->triangle->surfaceCharacteristic;
    default:
        return (float3)0.f;
    }
}
//************************************** SCENE ****************************************************

//************************************** CAMERA ***************************************************
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
    float aspectRatio = resolution.x / (float)resolution.y;

    float3 wVector = normalize(-viewDirection);
    float3 uVector = normalize(cross(up, wVector));
    float3 vVector = normalize(cross(wVector, uVector));

    float horizontalFactor = tan(camera->fov.x * 0.5f * (M_PI_F / 180)) * aspectRatio;
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
//************************************** CAMERA ***************************************************

//************************************** PATHTRACER ***********************************************
//####################################  DEFINES  ##################################################
#ifndef N_BOUNCES
#define N_BOUNCES 5
#endif

#define DEFAULT 0
#define NORMAL 1
#define DEPTH 2
#define COLOR 3
#define EMITTANCE 4
#define OBJECT_ID 5
#define SURFACE_CHARACTERISTIC 6
#define RANDOM 7

//####################################  DECLARATIONS  #############################################
float3 showDebugVision(Intersection *intersection, int options, float rand0, float rand1, float rand2);
float3 nextEventEstimation(Scene *scene, Intersection *intersection, float rand0, float rand1, float rand2);

//####################################  IMPLEMENTATIONS  ##########################################
float3 showDebugVision(Intersection *intersection, int options, float rand0, float rand1, float rand2)
{
    if (options == NORMAL)
    {
        return (intersection->normal + (float3)(1.f)) * 0.5f;
    }
    else if (options == DEPTH)
    {
        return (float3)intersection->t / 5.f;
    }
    else if (options == COLOR)
    {
        return getObjectColor(intersection);
    }
    else if (options == EMITTANCE)
    {
        return getObjectEmittance(intersection);
    }
    else if (options == OBJECT_ID)
    {
        if (intersection->objectType == SPHERE)
        {
            return (float3)intersection->sphereId / 15.f;
        }
        else if (intersection->objectType == TRIANGLE)
        {
            return (float3)intersection->triangleId / 15.f;
        }
    }
    else if (options == SURFACE_CHARACTERISTIC)
    {
        return (getObjectSurfaceCharacteristic(intersection) + 1.f) * 0.5f;
    }
    else if (options == RANDOM)
    {
        return (float3)(rand0, rand1, rand2);
    }

    return (float3)0.f;
}

float3 nextEventEstimation(Scene *scene, Intersection *intersection, float rand0, float rand1, float rand2)
{
    float3 directLight = (float3)1.f;

    float lightSelector = rand0;
    __constant Sphere *selectedLightSphere;
    __constant Triangle *selectedLightTriangle;
    int selectedLightObjectType;
    int selectedLightId = -1;

    float radianceContribution = 0.f;

    for (int i = 0; i < scene->lightSphereCount; i++)
    {
        __constant LightSphere *currentLightSphere = &scene->lightSpheres[i];
        float currentRadianceContribution = currentLightSphere->radiance / scene->totalRadiance;
        radianceContribution += currentRadianceContribution;

        if (lightSelector < radianceContribution)
        {
            selectedLightId = currentLightSphere->sphereId;
            selectedLightSphere = &scene->spheres[selectedLightId];
            selectedLightObjectType = SPHERE;
            directLight /= currentRadianceContribution;
            break;
        }
    }

    if (selectedLightId == -1)
    {
        for (int i = 0; i < scene->lightTriangleCount; i++)
        {
            __constant LightTriangle *currentLightTriangle = &scene->lightTriangles[i];
            float currentRadianceContribution = currentLightTriangle->radiance / scene->totalRadiance;
            radianceContribution += currentRadianceContribution;

            if (lightSelector < radianceContribution)
            {
                selectedLightId = currentLightTriangle->triangleId;
                selectedLightTriangle = &scene->triangles[selectedLightId];
                selectedLightObjectType = TRIANGLE;
                directLight /= currentRadianceContribution;
                break;
            }
        }
    }

    if (selectedLightObjectType == SPHERE)
    {
        float3 directionToLight = normalize(selectedLightSphere->position - intersection->position);
        float distanceToPoint = INFINITY;

        Ray lightRay;

        if (selectedLightSphere->radius < EPSILON)
        { // point light source
            float3 vectorToPoint = selectedLightSphere->position - intersection->position;
            lightRay.direction = normalize(vectorToPoint);
            distanceToPoint = length(vectorToPoint);

            directLight /= (4 * M_PI_F);                                // radiance contribution of point into one direction
            directLight /= pow(distanceToPoint, 2.f);                   // distance falloff
            directLight *= dot(intersection->normal, directionToLight); // cos theta
        }
        else
        {
            float3 hemisphereSample = sampleCosHemisphere(0.f, rand1, rand2);
            float3 pointOnLightSphere = selectedLightSphere->position + transformIntoTangentSpace(-directionToLight, hemisphereSample) * selectedLightSphere->radius;

            lightRay.direction = normalize(pointOnLightSphere - intersection->position);

            float3 normalAtLightPoint = normalize(pointOnLightSphere - selectedLightSphere->position);

            directLight /= 2 * M_PI_F * pow(selectedLightSphere->radius, 2.f);                                           // pdf - surface area half
            directLight /= pow(length(intersection->position - pointOnLightSphere), 2.f);                                // distance falloff
            directLight *= dot(intersection->normal, lightRay.direction) * dot(normalAtLightPoint, -lightRay.direction); // cos theta i and j
        }

        lightRay.origin = intersection->position + intersection->normal * EPSILON;

        Intersection lightIntersection;

        if ((findIntersection(&lightRay, &lightIntersection, scene) && lightIntersection.objectType == SPHERE && lightIntersection.sphereId == selectedLightId) || distanceToPoint < lightIntersection.t)
        { // first intersection is with selected light source
            directLight *= selectedLightSphere->emittance;
        }
        else
        {
            directLight = 0.f;
        }
    }
    else if (selectedLightObjectType == TRIANGLE)
    {
        float3 pointOnTriangle = sampleUniformPointOnTriangle(selectedLightTriangle, rand1, rand2);
        float triangleArea = getTriangleArea(selectedLightTriangle);

        Ray lightRay;
        lightRay.origin = intersection->position + intersection->normal * EPSILON;
        lightRay.direction = normalize(pointOnTriangle - intersection->position);

        Intersection lightIntersection;

        if (findIntersection(&lightRay, &lightIntersection, scene) && lightIntersection.objectType == TRIANGLE && lightIntersection.triangleId == selectedLightId)
        {
            directLight /= triangleArea;                                             // pdf for point on triangle
            directLight /= pow(length(pointOnTriangle - intersection->position), 2); // distance falloff
            directLight *= dot(intersection->normal, lightRay.direction);            // cos
            directLight *= selectedLightTriangle->emittance;                         // Le
        }
        else
        {
            directLight = 0.f;
        }
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
    scene.spheres = spheres;
    scene.lightSpheres = lightSpheres;
    scene.triangles = triangles;
    scene.lightTriangles = lightTriangles;

    int position = gy * width + gx;

    Ray ray = getCameraRay(camera, gx, gy, iteration, seed);

    float3 L = (float3)(0.f);
    float3 brdfCosFactor = (float3)(1.f);

    float randSeed = seed;

    for (int i = 0; i < N_BOUNCES; i++)
    {
        Intersection intersection;

        if (findIntersection(&ray, &intersection, &scene))
        {
            float rand0 = random((gx + iteration) / (float)width, gy / (float)height, randSeed);
            float rand1 = random(gx / (float)width, (gy + iteration) / (float)height, randSeed);
            float rand2 = random(gx / (float)width, gy / (float)height, randSeed + iteration);

            if (options != DEFAULT)
            {
                L = showDebugVision(&intersection, options, rand0, rand1, rand2);

                break;
            }

            float3 emittedLight = getObjectEmittance(&intersection);
            L += emittedLight * brdfCosFactor;

            brdfCosFactor *= getObjectColor(&intersection);

            float3 directLight = nextEventEstimation(&scene, &intersection, rand0, rand1, rand2);
            L += directLight * brdfCosFactor;

            float3 newDirection;
            float rand3 = random((gx + iteration) / (float)width, (gy + iteration) / (float)height, randSeed); // select which surface characteristic is used for the next ray direction
            float3 selectedSurfaceCharacteristic = getObjectSurfaceCharacteristic(&intersection);

            if (rand3 < selectedSurfaceCharacteristic.x)
            { // diffuse reflection
                float rand4 = random((gx + randSeed) / (float)width, (gy + iteration) / (float)height, randSeed);
                float rand5 = random((gx + iteration) / (float)width, (gy + randSeed) / (float)height, randSeed);
                float3 directionInHemisphere = sampleCosHemisphere(0.f, rand4, rand5);

                newDirection = transformIntoTangentSpace(intersection.normal, directionInHemisphere);
            }
            else if (rand3 < selectedSurfaceCharacteristic.x + selectedSurfaceCharacteristic.y)
            { // specular reflection
                newDirection = reflect(&ray, intersection.normal);
            }
            else
            {
                break;
            }

            ray.origin = intersection.position + intersection.normal * EPSILON;
            ray.direction = newDirection;

            brdfCosFactor *= dot(intersection.normal, ray.direction);

            randSeed = random(randSeed, position / (width), i * seed);
        }
        else
        {
            L += sceneInfo->backgroundColor * brdfCosFactor;
            break;
        }
    }

    image[position] = (image[position] * iteration + (float4)(L.xyz, 1.f)) / (iteration + 1.f);
}
//************************************** PATHTRACER ***********************************************

__kernel void clear(__global float4 *image, const int width)
{
    image[get_global_id(1) * width + get_global_id(0)] = (float4)(0.f);
}