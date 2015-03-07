// Library includes
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>

// Custom includes
#include "Globals.h"
#include "Tetrahedron.h"
#include "Cell.h"
#include "Moves.h"
#include "MCDriver.h"

using namespace std;

int output_count = 0;

void PrintOutput(MCDriver &driver)
{
    char buff[100];
    sprintf(buff, "%04d", output_count++);
    string fname = string("out_") + string(buff);

    ofstream f;
    f.open(fname);
    f << driver.ToString();
    f.close();
}

Real PackingFraction(MCDriver &driver)
{
    return driver.particles.size() * 1.0 / (6*sqrt(2.0)) / driver.cell.GetVolume();
}

int main()
{
    srand(time(NULL));

    // Some parameters
//    Real dtheta = 2*PI/10.;
//    Real dcell = .1;

    // Create the driver
    MCDriver driver(5);
    // Change some parameters
    driver.p_cell_move = 0.1;
    driver.BetaP=100000;
    driver.Beta=0;
    driver.SetCellShapeDelta(0.05);

/*
    for(int i=0;i<5;i++)
    {
        driver.cell.h(0, 0) -= 0.2;
        
        vector<Tetrahedron*> ghosts = driver.GetPeriodicGhosts();
        driver.CheckCollisionsWith(driver.particles[0], ghosts);
        MCDriver::__FreeGhosts(ghosts);

        PrintOutput(driver);
    }
        cout << 1/0 << endl;
*/

    int total = 150000;
    for(int i=0;i<total;i++)
    {
        // Print out every few steps
        if(i%(total/100)==0)
        {
            cout << "Volume = " << driver.cell.GetVolume() << endl;
            cout << "Fraction = " << PackingFraction(driver) << endl;
            PrintOutput(driver);
        }

        driver.MakeMove();
    }

    return 0;
}
