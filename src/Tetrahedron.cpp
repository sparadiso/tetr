#include "Tetrahedron.h"

// ========================================================================================================
// Constructor - build a new Tetrahedron instance from a point (the COM) + 3D rotation
// ========================================================================================================
Tetrahedron::Tetrahedron(Vector origin, float roll, float pitch, float yaw): Shape()
{
    //Start from a standard Tetrahedron centered at (0,0,0), then apply the origin translation and rotation
    vertices.push_back(Vector(0.5, 0, -0.5/sqrt(2.0)));
    vertices.push_back(Vector(-0.5, 0, -0.5/sqrt(2.0)));
    vertices.push_back(Vector(0, 0.5, 0.5/sqrt(2.0)));
    vertices.push_back(Vector(0, -0.5, 0.5/sqrt(2.0)));

    this->Translate(origin);
    this->Rotate(roll, pitch, yaw);
}

// ========================================================================================================
// Intersects - Returns `true` if the two Tetrahedra (`this` and `t2`) are intersecting, `false` otherwise
// ========================================================================================================
bool Tetrahedron::Intersects(Shape *t2)
{
    //TODO: Implement
    if (sqrt((this->GetCOM() - t2->GetCOM()).norm()) < 1)
        return true;

    return false;
}
