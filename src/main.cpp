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
void PrintOutput(string str, MCDriver<T> &d);

// Output
int output_count = 0;

// Choose shape to use - valid choices: {Tetrahedron, Sphere}
typedef Tetrahedron ChosenShape;

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
    int n_particles = GetParameter("n_particles", 2);
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
        d->SetParticleTranslationDelta(GetParameter("dr", 0.1));
        d->Project_Threshold = GetParameter("ProjectionThreshold", 0.3);
        
        drivers.push_back(d);
    }

    // Run the simulation
    if(GetParameter("Debug", 0) < .9)
        RunProduction();
    else
        RunDebug();

    return 0;
}

// Make each move and undo it to make sure the moves are doing what they ought to
void ParticleMoveTest(MCDriver<ChosenShape> *driver)
{
    driver->p_cell_move = 0.0;

    Cell *cell = &driver->cell;
/*
    cell->h.setIdentity();
    cell->h *= sqrt(driver->particles.size());
*/

    int total = GetParameter("DebugSteps", 2000);
    for(int i=0;i<total;i++)
    {
        if(driver->MakeMove())
            cout << "Accepted" << endl;
        else
            cout << "Rejected" << endl;
        PrintOutput("dbg",*driver);
    }
}

// Check tetrahedra collision
void TetrahedraCollideTest(MCDriver<ChosenShape> *driver)
{
    // Initialize one tetrahedron at the origin    
    Shape *t = driver->particles[0];
    t->Translate(Vector(1.5,0,0)-t->GetCOM());

    t = driver->particles[1];
    t->Translate(-t->GetCOM());
    Vector dr(.9, 1.5, 0.8);
    t->Translate(dr);

    Shape *p0 = driver->particles[0];
    Shape *p1 = driver->particles[1];

    // Move the first particle towards the second in 10 steps
    dr = Vector(dr - p0->GetCOM());
    dr /= 10;
    for(int i=0;i<10;i++)
    {
        p0->Translate(dr);

        if(p0->Intersects(p1))
            cout << i << ": Intersecting" << endl;
        else
            cout << i << ": Not Intersecting" << endl;

        PrintOutput("ParticleMove", *driver);
    }
}

// Make each move and undo it to make sure the moves are doing what they ought to
void ApplyUndoTest(MCDriver<ChosenShape> *driver)
{
    for(uint i=0;i<driver->cell_moves.size();i++)
    {   
        Move *m = driver->cell_moves[i];
        PrintOutput("CellMove", *driver);
        m->Apply();
        PrintOutput("CellMove", *driver);
        m->Undo();
        PrintOutput("CellMove", *driver);
    }

    for(uint i=0;i<driver->particle_moves.size();i++)
    {   
        Move *m = driver->cell_moves[i];
        PrintOutput("ParticleMove", *driver);
        m->Apply();
        PrintOutput("ParticleMove", *driver);
        m->Undo();
        PrintOutput("ParticleMove", *driver);
    }
}

void RunDebug()
{
    MCDriver<ChosenShape> *driver = drivers[0];
    ParticleMoveTest(driver);
//    TetrahedraCollideTest(driver);
}

void RunProduction()
{
    // Now run the MC Simulation
    int total = GetParameter("n_steps", 150000);
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
                
                // Take this opportunity to update the step sizes
                cout << "Cell Accept Ratio: ";
                for(uint k=0;k<drivers[j]->cell_moves.size();k++)
                    cout << drivers[j]->cell_moves[k]->GetRatio() << ", ";
                cout << endl;
                if(GetParameter("UpdateMoveSizeYN", 0) > .99)
                    drivers[j]->UpdateMoveSizes(0.3);

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
                    PrintOutput("best", *drivers[j]);
                    BestSolutionPrinted = pack_fraction;
                }   
                BestSolution = pack_fraction;
            }
        }
    }

    cout << "FINISHED - Best Solution: " << BestSolution << endl;
    PrintOutput("best", *drivers[0]);
}

template <class T>
void PrintOutput(string str, MCDriver<T> &driver)
{
    char buff[100];
    sprintf(buff, "%04d", output_count);
    output_count++;
    string fname = "output/" + str + string(buff);

    ofstream f;
    f.open(fname);
    f << driver.ToString(true);
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
