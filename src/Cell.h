#pragma once

#include "Globals.h"
#include "Shape.h"

class Cell
{
    public:
    // Cell tensor
    Matrix h;
    std::vector<Shape*> particles;

    // Constructor
    Cell(int n);

    // Instance Methods
    Real GetVolume();
    Vector PartialCoords(Vector v);
    Vector PeriodicImage(Vector v);
    std::string ToString();
    void WrapShape(Shape *t);
};
