#ifndef MCDRIVER_H_
#define MCDRIVER_H_

#include "Globals.h"
#include "Cell.h"
#include "Tetrahedron.h"
#include "Moves.h"

class MCDriver
{
    public:

    Cell cell;
    std::vector<Tetrahedron*> particles;

    // List of Moves we'll use (particle vs cell)
    std::vector<Move*> moves;

    // Move parameters
    Real dv_max; // Maximum cell volume move
    Real dr_max; // Maximum particle displacement move
    Real droll_max, dpitch_max, dyaw_max; // Maximum delta rotations along each axis

    // Maintain runtime statistics on move acceptance/rejection rate
    int moves_accepted[N_MOVES], moves_attempted[N_MOVES];

    MCDriver(int n_particles, Real dr, Real droll, Real dpitch, Real dyaw,
                                        Real dx, Real dy, Real dz);

    void MakeMove();
    void PerformTranslationMove();
    void PerformRotationMove();

    bool CheckCollisionsWith(Tetrahedron *t, std::vector<Tetrahedron*> ghosts);
    void PerformCellVolumeMove(Real dv);
    void PerformCellShapeMove(Real dv);
    std::vector<Tetrahedron*> GetPeriodicGhosts();
    std::string ToString();
};

#endif
