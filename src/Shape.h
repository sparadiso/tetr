#pragma once
#include "Globals.h"

class Shape
{
    private:
    void Rotate(Matrix rot_matrix);

    public:
    std::vector<Vector> vertices;
    std::vector<Shape*> periodic_images;

    // Member methods
    Vector GetCOM();
    static Matrix GetRotationMatrix(float roll, float pitch, float yaw);

    virtual void Rotate(float roll, float pitch, float yaw);
    virtual void Translate(Vector v);

    std::string ToString();

    // Virtual destructor
    virtual ~Shape();

    // Abstract methods to be implemented in Sphere/Tetrahedron
    virtual bool Intersects(Shape *s) = 0;
    virtual Real GetVolume() = 0;
};
