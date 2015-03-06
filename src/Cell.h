#ifndef CELL_H_
#define CELL_H_

#include "Globals.h"

class Cell
{
    public:
    // Cell tensor
    Matrix h;

    Cell();

//    void Update();
    Vector PeriodicImage(Vector &v);
    std::string ToString();
};

#endif
