
#include "Globals.h"

Real u(Real lower, Real upper)
{
    // Uniform random number on (0, 1)
    Real ret = (rand() % 10000) / 10000.0;
    ret *= upper - lower;
    ret += lower;

    return ret;
}

