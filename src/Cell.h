#ifndef CELL_H_
#define CELL_H_

#include "Globals.h"
#include "Shape.h"

class Cell
{
    public:
    // Cell tensor
    Matrix h;

    // Constructor
    Cell(int n);

    // Methods
    Real GetVolume();
    Vector PartialCoords(Vector v);
    Vector PeriodicImage(Vector v);
    std::string ToString();
    void WrapShape(Shape *t);
};

#endif
