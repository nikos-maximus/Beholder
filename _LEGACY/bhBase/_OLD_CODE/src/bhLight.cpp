#include "bhLight.h"
#include <math.h>
#define LIGHT_THRESHOLD 0.001f

// Attenuation of spherical light
// fAtt = 1 / (((distance/radius) + 1)^2)

inline float CalculateArea(float intensity, float radius)
{
    // See also : https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
    return radius * (sqrtf(intensity / LIGHT_THRESHOLD) - 1.f);
}

bhLight::bhLight()
    : color(1.f, 1.f, 1.f, 1.f)
{
    effectArea = CalculateArea(intensity, sourceRadius);
}

void bhLight::SetRadius(float newRadius)
{
    sourceRadius = newRadius;
    effectArea = CalculateArea(intensity, sourceRadius);
}

void bhLight::SetIntensity(float newIntensity)
{
    intensity = newIntensity;
    effectArea = CalculateArea(intensity, sourceRadius);
}
