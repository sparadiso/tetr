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

void RunProduction();
void RunDebug();

template <class T>
void PrintOutput(MCDriver<T> &d);

// Output
int output_count = 0;

// Choose shape to use - valid choices: {Tetrahedron, Sphere}
typedef Sphere ChosenShape;

// Some globals
vector< MCDriver<ChosenShape>* > drivers;

// Command line arg parsing
vector<string> keys;
vector<Real> values;
Real GetParameter(string p, Real def = -1234321);

// Main Declaration
int main(int argc, char* argv[])
{
    for(int i=1;i<argc;i+=2)
    {
        keys.push_back(string(argv[i]));
        values.push_back(atof(argv[i+1]));
    }

    srand(time(NULL));

    // Create the subsystems for the parallel tempering scheme
    // Each subsystem will be at a different pressure, with 
    // periodic pressure swap moves between systems to get out of local minima
    // This typically works better than simulated annealing (ie. slow pressure ramp)
    int n_drivers = GetParameter("n_drivers", 1);
    Real *pressures = new Real[n_drivers];
    for(int i=0;i<n_drivers;i++)
    {
        pressures[i] = GetParameter(string("p")+to_string(i), 100);
    }

    // Options
    int n_particles = GetParameter("n_particles", 1);
    Real p_cell_move = GetParameter("p_cell_move", 1.0/(n_particles+1));
    //Real cell_shape_delta = 0.05;
        
    // Construct the subsystems and add them to the `drivers` list
    for(int i=0;i<n_drivers;i++)
    {
        MCDriver<ChosenShape> *d = new MCDriver<ChosenShape>(n_particles, p_cell_move);

        // Set options
        d->BetaP = pressures[i];
        d->Beta = GetParameter("potential", 0);
        d->SetCellShapeDelta(GetParameter("dcell", 0.1));
        d->SetCellVolumeDelta(GetParameter("dv", 0.1));
        d->SetParticleTranslationDelta(GetParameter("dr", 0.1));
        
        drivers.push_back(d);
    }

    // Run the simulation
    RunProduction();
//    RunDebug();

    return 0;
}

void RunDebug()
{
    ChosenShape *particle = drivers[0]->particles[0];
    MCDriver<ChosenShape> *driver = drivers[0];
    Cell *cell = &(driver->cell);

    driver->p_cell_move = 1.0;

    cell->h.setIdentity();

    for(int i=0;i<100;i++)
    {
        driver->cell_moves[1]->Apply();
        cell->WrapShape(particle);
        cout << cell->GetVolume() << endl;
        PrintOutput(*driver);
    }
}

void RunProduction()
{
    // Now run the MC Simulation
    int total = GetParameter("N_Steps", 150000);
    Real BestSolution = 0;
    Real BestSolutionPrinted = 0;

    for(int i=0;i<total;i++)
    {
        // Do a parallel tempering swap 
        if(u(0,1) < 0.1 && drivers.size() > 1)
        {
            int sys0, sys1;
            sys0 = u(0,1)*drivers.size();
            sys1 = u(0,1)*drivers.size();
            while(sys1 == sys0)
                sys1 = u(0,1)*drivers.size();
            Real p0 = drivers[sys0]->BetaP;
            Real p1 = drivers[sys1]->BetaP;
            Real V0 = drivers[sys0]->cell.GetVolume();
            Real V1 = drivers[sys1]->cell.GetVolume();

            Real BoltzmannFactor = exp(-((p0-p1)*V1 + (p1-p0)*V0));
            if( u(0, 1) < BoltzmannFactor )
            {
                drivers[sys0]->BetaP = p1;
                drivers[sys1]->BetaP = p0;
            }
        }

        // Print out every few steps
        if(i%(total/(int)GetParameter("N_Write", 25))==0)
        {
            cout << "===== Step " << i << " Completed =====" << endl;
            cout << endl;
            cout << "Best Solution: " << BestSolution << endl;
            cout << endl;

            for(uint j=0;j<drivers.size();j++)
            {
                cout << "System " << j << " (P=" << drivers[j]->BetaP << ")" << endl;
                cout << "==========" << endl;
                cout << "Volume: " << drivers[j]->cell.GetVolume() << endl;
                cout << "Pack Fraction: " << drivers[j]->GetPackingFraction() << endl;
                
                // Take this opportunity to update the step sizes
                cout << "Cell Accept Ratio: " << drivers[j]->cell_moves[0]->GetRatio() << ", " << drivers[j]->cell_moves[1]->GetRatio() << endl;
                if(abs(GetParameter("UpdateMoveSizeYN", 0) - 1) < .0001)
                    drivers[j]->UpdateMoveSizes(0.1);

                cout << endl;
            }
        }

        // Take an MC Move in each subsystem
        for(uint j=0;j<drivers.size();j++)
        {
            drivers[j]->MakeMove();
        }

        // Keep track of the best solution
        for(uint j=0;j<drivers.size();j++)
        {
            Real pack_fraction = drivers[j]->GetPackingFraction();

            if ( BestSolution < pack_fraction )
            {
                // Only print it to a file if we've improved by at least 2%
                if(pack_fraction - BestSolutionPrinted > 0.05)
                {
                    PrintOutput(*drivers[j]);
                    BestSolutionPrinted = pack_fraction;
                }   
                BestSolution = pack_fraction;
            }
        }
    }

    cout << "FINISHED - Best Solution: " << BestSolution << endl;
    PrintOutput(*drivers[0]);
}

template <class T>
void PrintOutput(MCDriver<T> &driver)
{
    char buff[100];
    sprintf(buff, "%04d", output_count++);
    string fname = string("out_") + string(buff);

    ofstream f;
    f.open(fname);
    f << driver.ToString(false);
    f.close();
}

Real GetParameter(string param_name, Real default_value)
{
    for(uint i=0;i<keys.size();i++)
    {
        if (keys[i] == param_name)
            return values[i];
    }

    // If a default value was passed, then use it
    if ( abs(default_value - (-1234321)) > .00001 )
        return default_value;

    cout << "Error: Couldn't find a value for parameter " << param_name << " in cmd args" << endl;
    exit(1);
}
