#pragma once
#include "Globals.h"

class Shape
{
    public:
    std::vector<Vector> vertices;

    // Member methods
    Vector GetCOM();
    static Matrix GetRotationMatrix(float roll, float pitch, float yaw);
    void Rotate(float roll, float pitch, float yaw);
    void Rotate(Matrix rot_matrix);
    std::string ToString();
    void Translate(Vector v);

    // Virtual destructor
    virtual ~Shape();

    // Abstract methods to be implemented in Sphere/Tetrahedron
    virtual bool Intersects(Shape *s) = 0;
    virtual Real GetVolume() = 0;
};
