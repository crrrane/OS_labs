#include "lib.h"
#include <math.h>

float cos_derivative(float a, float dx)
{
    return (cosf(a + dx) - cosf(a - dx)) / (2 * dx);
}

float area(float a, float b)
{
    return 0.5 * a * b;
}