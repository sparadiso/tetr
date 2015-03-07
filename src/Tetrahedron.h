#pragma once
#include "Shape.h"

// Defines a tetrahedron that lives in standard Cartesian space (orthogonal x,y,z bases) independent of unit cell shape. 
// All issues associated with the cell tensor (including collision across PBCs) are handled in MCDriver.cpp
class Tetrahedron: public Shape
{
    public:
    
    //Constructor
    Tetrahedron(Vector origin, float roll=0, float pitch=0, float yaw=0);

    // Implement intersection with other Tetrahedra (if this were an actual library, of course, this would have to work between any two Shapes).
    // Here, we just assume t2 is another Tetrahedron.
    bool Intersects(Shape *t2);
};
