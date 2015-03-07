#include "Sphere.h"

Sphere::Sphere(Vector v)
{
    this->vertices.push_back(v);
}

bool Sphere::Intersects(Shape *s2)
{
    if (sqrt((this->GetCOM() - s2->GetCOM()).norm()) < 1)
        return true;

    return false;
}
