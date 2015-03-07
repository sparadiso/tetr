#pragma once
#include "Globals.h"
#include "Shape.h"

class Sphere: public Shape
{
    public:
    // Constructor
    Sphere(Vector origin);

    // Implement sphere-sphere intersection
    bool Intersects(Shape *t2);
};
