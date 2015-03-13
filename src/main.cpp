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

// Choose shape to use - valid choices: {Tetrahedron, Sphere}
typedef Tetrahedron ChosenShape;

// Output
int output_count = 0;
Real BestSolution = 0;
Real BestSolutionPrinted = 0;

template <class T>
void PrintOutput(string str, MCDriver<T> &d);
template <class T>
MCDriver<T>* GetBestDriver(vector<MCDriver<T>*> drivers);

// Command line arg parsing
vector<string> keys;
vector<Real> values;
Real GetParameter(string p, Real def = -1234321);

template <class T>
void RunProduction(vector<MCDriver<T>*> drivers);

// Main Declaration
int main(int argc, char* argv[])
{
    // Parse cmdline args
    for(int i=1;i<argc;i+=2)
    {
        keys.push_back(string(argv[i]));
        values.push_back(atof(argv[i+1]));
    }

    // Initialize the RNG
    srand(time(NULL));

    // Create the subsystems for the parallel tempering scheme
    // Each subsystem will be at a different pressure, with 
    // periodic pressure swap moves between systems to get out of local minima
    // This typically works better than simulated annealing (ie. slow pressure ramp)
    int n_drivers = GetParameter("n_drivers", 1);

    // Driver Options
    int n_particles = GetParameter("n_particles", 2);
    Real p_cell_move = GetParameter("p_cell_move", 1.0/(n_particles+1));
        
    // Construct the subsystems and add them to a driver list
    vector< MCDriver<ChosenShape>* > drivers;
    for(int i=0;i<n_drivers;i++)
    {
        MCDriver<ChosenShape> *d = new MCDriver<ChosenShape>(n_particles, p_cell_move);

        // Set options
        d->BetaP = GetParameter(string("p")+to_string(i), 100);
        d->SetCellShapeDelta(GetParameter("dcell", 0.02));
        d->SetParticleTranslationDelta(GetParameter("dr", 0.02));
        d->Project_Threshold = GetParameter("ProjectionThreshold", 0.65);
        
        drivers.push_back(d);
    }

    RunProduction(drivers);
    return 0;
}

// ============================================================================
// Run the actual MC simulation
// ============================================================================
template<class T>
void RunProduction(vector< MCDriver<T>* > drivers)
{
    MCDriver<T>* best;

    // Now run the MC Simulation
    int total = GetParameter("n_steps", 10000000);

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

                drivers[sys0]->cell_moves[0]->Reset();
                drivers[sys1]->cell_moves[0]->Reset();
            }
        }

        // Print out every few steps
        if(i%(total/(int)GetParameter("n_write", 25))==0)
        {
            cout << "===== Step " << i << " Completed =====" << endl;
            cout << endl;
            cout << "Best Solution: " << BestSolution << endl;
            cout << endl;
    
            for(uint j=0;j<drivers.size();j++)
                PrintOutput(string("Driver_")+to_string(j)+string("_"), *drivers[j]);
            
            for(uint j=0;j<drivers.size();j++)
            {
                cout << "System " << j << " (P=" << drivers[j]->BetaP << ")" << endl;
                cout << "==========" << endl;
                cout << "Volume: " << drivers[j]->cell.GetVolume() << endl;
                cout << "Pack Fraction: " << drivers[j]->GetPackingFraction() << endl;
                
                // Print the cell move acceptance rate for each pressure to guide future choices
                cout << "Cell Accept Ratio: ";
                for(uint k=0;k<drivers[j]->cell_moves.size();k++)
                    cout << drivers[j]->cell_moves[k]->GetRatio() << ", ";
                cout << endl;
                cout << endl;
            }
        }

        // Take an MC Move in each subsystem
        for(uint j=0;j<drivers.size();j++)
        {
            drivers[j]->MakeMove();
        }

        // Keep track of the best solution over time
        best = GetBestDriver(drivers);
        Real best_fraction = best->GetPackingFraction();

        // Only print it to a file if we've improved by at least 1%
        if(best_fraction - BestSolutionPrinted > 0.01)
        {
            PrintOutput("best", *best);
            BestSolutionPrinted = best_fraction;
        }   

        if (best_fraction > BestSolution)
            BestSolution = best_fraction;
    }

    cout << "FINISHED - Best Solution: " << BestSolution << endl;
}

// ============================================================================
// A few helper functions
// ============================================================================
template <class T>
void PrintOutput(string str, MCDriver<T> &driver)
{
    char buff[100];
    sprintf(buff, "%04d", output_count);
    output_count++;
    string fname = "output/" + str + string(buff);

    ofstream f;
    f.open(fname);
    f << driver.ToString();
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

template <class T>
MCDriver<T>* GetBestDriver(vector<MCDriver<T>*> drivers)
{
    MCDriver<T>* best;
    Real best_fraction = 0; 

    for(uint j=0;j<drivers.size();j++)
    {
        Real pack_fraction = drivers[j]->GetPackingFraction();

        if ( best_fraction < pack_fraction )
        {
            best_fraction = pack_fraction;
            best = drivers[j];
        }
    }
    return best;
}

