// Library includes
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>

// Custom includes
#include "Globals.h"
#include "Tetrahedron.h"
#include "Sphere.h"
#include "Cell.h"
#include "Moves.h"
#include "MCDriver.h"

using namespace std;

// Output
int output_count = 0;

template <class T>
void PrintOutput(MCDriver<T> &driver)
{
    char buff[100];
    sprintf(buff, "%04d", output_count++);
    string fname = string("out_") + string(buff);

    ofstream f;
    f.open(fname);
    f << driver.ToString();
    f.close();
}

// Quick helper to compute the packing fraction of a system
template <class T>
Real PackingFraction(MCDriver<T> &driver)
{
    return driver.particles.size() * driver.particles[0]->GetVolume() / driver.cell.GetVolume();
}

typedef Sphere ChosenShape;

int main()
{
    srand(time(NULL));

    // Create the subsystems for our parallel tempering scheme
    vector< MCDriver<ChosenShape>* > drivers;
    int n_drivers = 1;
    Real pressures[1] = {1000};

    // Options
    int n_particles = 3;
    Real p_cell_move = 1.0 / (n_particles+1);
    Real cell_shape_delta = 0.05;
    Real Beta = 0;
        
    for(int i=0;i<n_drivers;i++)
    {
        MCDriver<ChosenShape> *d = new MCDriver<ChosenShape>(n_particles, p_cell_move);

        // Set options
        d->BetaP = pressures[i];
        d->Beta = Beta;
        d->SetCellShapeDelta(cell_shape_delta);
        
        drivers.push_back(d);
    }

    // Now run the MC Simulation
    int total = 1500000;
    for(int i=0;i<total;i++)
    {
        // Print out every few steps
        if(i%(total/100)==0)
        {
            for(uint j=0;j<drivers.size();j++)
            {
                cout << "Driver " << j << endl;
                cout << "==========" << endl;
                cout << "Volume: " << drivers[j]->cell.GetVolume() << endl;
                cout << "Pack Fraction: " << PackingFraction(*drivers[j]) << endl;
            }

            MCDriver<ChosenShape> *best = drivers[0];
            for(uint j=0;j<drivers.size();j++)
                if ( PackingFraction(*best) < PackingFraction(*drivers[j]) )
                    best = drivers[j];

            PrintOutput(*best);
        }

        for(uint j=0;j<drivers.size();j++)
        {
            drivers[j]->MakeMove();
        }
    }

    return 0;
}
