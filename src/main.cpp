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

void PrintOutput(string fname, MCDriver &driver)
{
    ofstream f;
    f.open(fname);
    f << driver.ToString();
    f.close();
}

int main()
{
    srand(time(NULL));

    // Some parameters
//    Real dtheta = 2*PI/10.;
//    Real dcell = .1;

    // Create the driver
    MCDriver driver(1);
    // Change some parameters
    driver.p_cell_move = 1.0;
    driver.BetaP=1.0;

    int output_count = 0;
    for(int i=0;i<1000;i++)
    {
        cout << driver.cell.GetVolume() << endl;
        // Print out every few steps
        if(i%10==0)
        {
            char buff[100];
            sprintf(buff, "%04d", output_count++);
            string fname = string("out_") + string(buff);
            PrintOutput(fname, driver);
        }

        driver.MakeMove();
    }

    return 0;
}
