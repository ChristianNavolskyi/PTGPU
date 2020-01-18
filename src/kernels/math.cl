#ifndef MATH_H
#define MATH_H

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

void createOrthonormalAxes(float3 z, float3 *x, float3 *y) {
    float3 helpAxis = fabs(z.x) > 0.1f ? (float3) (0.f, 1.f, 0.f) : (float3) (1.f, 0.f, 0.f);

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


#endif