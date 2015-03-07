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
    std::vector<ParticleMove*> particle_moves;
    std::vector<CellMove*> cell_moves;

    // Thermo parameters
    Real BetaP, Beta;

    // Move parameters
    Real p_cell_move; // Probability of choosing a cell move (shape or volume) vs single particle move

    MCDriver(int n_particles, Real p_cell_move = 0.1, Real dr_particle=0.1, Real dtheta_particle=0.2, Real dcell = 0.02);

    static void __FreeGhosts(std::vector<Tetrahedron*> ghosts);

    bool CheckCollisionsWith(Tetrahedron *t, std::vector<Tetrahedron*> ghosts);
    std::vector<Tetrahedron*> GetPeriodicGhosts();
    void MakeMove();
    void SetCellShapeDelta(Real delta);
    std::string ToString();
};

#endif
