#include "Cell.h"

Cell::Cell(int n_particles)
{
    this->h.setIdentity();
    this->h *= n_particles;
}

Real Cell::GetVolume()
{
    // Return the volume of the cell (det(h))
    return std::abs(this->h.determinant());
}

Vector Cell::PartialCoords(Vector v)
{
    Vector s = this->h.colPivHouseholderQr().solve(v);
    return s;
}

// Return the new position vector corresponding to wrapping the input vector around the periodic box.
void Cell::WrapShape(Shape *t)
{
    // Wrap the shape back to within the bounds of the cell
    Vector old_com = t->GetCOM();
    Vector com = this->PeriodicImage(t->GetCOM());
    Vector dr = com - old_com;

    t->Translate(dr);
}
Vector Cell::PeriodicImage(Vector v)
{
    // First, solve h.s = v where s is the transformed, fractional coordinates of the input vector in the unit cell basis (with cell tensor 'h')
    Vector s = this->PartialCoords(v);

    for(uint i=0;i<3;i++)
    {
        while (s[i] < 0)
            s[i] += 1;
        s[i] = s[i] - int(s[i]);
    }

    return this->h * s;
}

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
