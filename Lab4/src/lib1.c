#include "lib.h"
#include <math.h>

float cos_derivative(float a, float dx)
{
    return (cosf(a + dx) - cosf(a)) / dx;
}

float area(float a, float b)
{
    return a * b;
}