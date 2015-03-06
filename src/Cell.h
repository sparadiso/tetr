#ifndef CELL_H_
#define CELL_H_

#include "Globals.h"

class Cell
{
    public:
    // Cell tensor
    Matrix h;

    // Constructor
    Cell();

    // Methods
    Real GetVolume();
    Vector PeriodicImage(Vector &v);
    std::string ToString();
};

#endif
