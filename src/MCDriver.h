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
    Real BetaP;

    // Move parameters
    Real p_cell_move; // Probability of choosing a cell move (shape or volume) vs single particle move

    MCDriver(int n_particles, Real p_cell_move = 0.1, Real dr=0.1, Real dtheta_particle=0.2, Real dtheta_cell = 0.2, Real dv=0.1);

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
