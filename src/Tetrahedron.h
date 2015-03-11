#pragma once
#include "Shape.h"

class Tetrahedron: public Shape
{
    public:
    
    Tetrahedron(Vector origin, float roll=0, float pitch=0, float yaw=0);

    bool Intersects(Shape *t2);
    Real GetVolume();
};
