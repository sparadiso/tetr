#include "Cell.h"
#include <iostream>

Cell::Cell()
{
    this->h.setIdentity();
}

// Return the new position vector corresponding to wrapping the input vector around the periodic box.
Vector Cell::PeriodicImage(Vector &v)
{
    // First, solve h.s = v where s is the transformed, fractional coordinates of the input vector in the unit cell basis (with cell tensor 'h')
    Vector s = this->h.colPivHouseholderQr().solve(v);
    //std::cout << "h" << this->h << std::endl;
    //std::cout << "v" << v << std::endl;
    //std::cout << "s" << s << std::endl;

    for(uint i=0;i<3;i++)
    {
        if(s[i] > 1)
        {
            s[i] = fmod(s[i], 1);
        }
        while (s[i] < 0)
            s[i] += 1;
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
/*
void Cell::Update()
{
    this->h_inverse = this->h.inverse();
}
*/
