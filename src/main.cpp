// Library includes
#include <string>
#include <iostream>
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
    FILE *f = open(fname.c_str());
    fprintf("%s", driver.ToString());
    close(f);
}

int main()
{
    srand(time(NULL));

    // Some parameters
    Real dtheta = 2*PI/10.;
    Real dcell = .1;

    // Create the driver
    MCDriver driver(1, .1, dtheta, dtheta, dtheta, dcell, dcell, dcell);
    
    PrintOutput(string("out_0"), driver);
    driver.MakeMove();
    PrintOutput(string("out_1"), driver);

    return 0;
}
