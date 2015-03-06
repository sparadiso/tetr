#ifndef MOVES_H_
#define MOVES_H_

#include "Globals.h"
#include "Cell.h"
#include "Tetrahedron.h"

enum MoveID { PARTICLE_TRANSLATION, PARTICLE_ROTATION, CELL_VOLUME, CELL_SHAPE,
             N_MOVES };

// These classes help simplify the logic of applying/undoing Monte Carlo moves. Generally speaking, this is probably overkill.
class Move
{
    public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Real delta_max;

    Move(Real delta_max);
    virtual void Apply() = 0;
    virtual void Undo() = 0;
};

// Particle moves
class ParticleMove: public Move
{
    public:
    Tetrahedron *particle;

    ParticleMove(Tetrahedron *t, Real delta_max);
};

class ParticleTranslation: public ParticleMove
{
    public:
    Vector *dr;

    ParticleTranslation(Tetrahedron *t, Real delta_max);
    void Apply();
    void Undo();
};

class ParticleRotation: public ParticleMove
{
    public:
    Matrix rot_inverse;

    ParticleRotation(Tetrahedron *t, Real delta_max);
    void Apply();
    void Undo();
};

// Cell moves
class CellMove: public Move
{
    public:
    Cell *cell;

    CellMove(Cell *c, Real delta_max);
};

class CellShapeMove: public CellMove
{
    public:
        
    // Last move info for Undo()
    Matrix h_old;

    CellShapeMove(Cell *c, Real delta_max);
    void Apply();
    void Undo();
};

#endif
