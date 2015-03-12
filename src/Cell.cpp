#include "Cell.h"

Cell::Cell(int n_particles)
{
    this->h.setIdentity();
    this->h *= n_particles;
}

// Return the volume of the cell (det(h))
Real Cell::GetVolume()
{
    return std::abs(this->h.determinant());
}

// Take a vector in R^3 and return its partial coordinates in the unit cell's reference (S^3: [{0,1}, {0,1}, {0,1}])
// v = h <dot> s 
Vector Cell::PartialCoords(Vector v)
{
    Vector s = this->h.colPivHouseholderQr().solve(v);
    return s;
}

// Translate the input Shape so that its center of mass is located within the fundamental cell (applying PBCs)
void Cell::WrapShape(Shape *t)
{
    Vector old_com = t->GetCOM();
    Vector com = this->PeriodicImage(t->GetCOM());
    Vector dr = com - old_com;

    t->Translate(dr);
}

// Return the wrapped representation of the input vector applying periodic boundaries
Vector Cell::PeriodicImage(Vector v)
{
    // First, solve h.s = v where s is the transformed, fractional coordinates of the input vector in the unit cell basis (with cell tensor 'h')
    Vector s = this->PartialCoords(v);

    for(uint i=0;i<3;i++)
    {
        s[i] = s[i] - int(s[i]);

        // Our fractional coordinates are defined on {0, 1}, not {-.5, .5}
        if (s[i] < 0)
            s[i] += 1;
    }

    return this->h * s;
}

// Spit out a formatted string representation of this fundamental cell (each line represents a basis vector)
std::string Cell::ToString()
{
    std::string s = "";

    for(int i=0;i<3;i++)
    {
        for(int j=0;j<3;j++)
            s += std::to_string(this->h(j,i)) + " ";
        if(i<2)
            s += "\n";
    }

    return s;
}
