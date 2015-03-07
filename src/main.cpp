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
    //return driver.particles.size() * 1.0 / (6*sqrt(2.0)) / driver.cell.GetVolume();
    Real r3 = .5*.5*.5;
    return driver.particles.size() * 4./3.*PI*r3 / driver.cell.GetVolume();
}

int main()
{
    srand(time(NULL));

    // Some parameters
//    Real dtheta = 2*PI/10.;
//    Real dcell = .1;

    // Create the driver
    MCDriver driver(8);
    // Change some parameters
    driver.p_cell_move = 0.2;
    driver.BetaP=10000;
    driver.Beta=0;
    driver.SetCellShapeDelta(0.05);

    int total = 1500000;
    for(int i=0;i<total;i++)
    {
        driver.BetaP = 1000. * i / total;
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
