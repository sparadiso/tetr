#include "Sphere.h"

Sphere::Sphere(Vector v)
{
    this->vertices.push_back(v);
}

Real Sphere::GetVolume()
{
    Real r3 = .5*.5*.5;
    return 4./3. * PI * r3;
}

bool Sphere::Intersects(Shape *s2)
{
    if (sqrt((this->GetCOM() - s2->GetCOM()).norm()) < 1)
        return true;

    return false;
}
