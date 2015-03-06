#ifndef TETRAHEDRON_H_
#define TETRAHEDRON_H_

#include "Globals.h"

// Defines a tetrahedron that lives in standard Cartesian space (orthogonal x,y,z bases) independent of unit cell shape. 
// All issues associated with the cell tensor (including collision across PBCs) are handled in MCDriver.cpp
class Tetrahedron
{
    public:
    std::vector<Vector> vertices;

    Tetrahedron(Vector origin, float roll=0, float pitch=0, float yaw=0);

    Vector GetCOM();
    bool Intersects(Tetrahedron *t2);
    Matrix Rotate(float roll, float pitch, float yaw);
    void Rotate(Matrix rot_matrix);
    std::string ToString();
    void Translate(Vector *v);
    void Translate(Vector &v);
};

#endif
