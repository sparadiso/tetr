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

    virtual void Apply() = 0;
    virtual void Undo() = 0;
};

// Particle moves
class ParticleMove: public Move
{
    public:
    Tetrahedron *particle;

    ParticleMove(Tetrahedron *t);
};

class ParticleTranslation: public ParticleMove
{
    public:
    Real dr_max;
    Vector *dr;

    ParticleTranslation(Tetrahedron *t, Real dr_max);
    void Apply();
    void Undo();
};

class ParticleRotation: public ParticleMove
{
    public:
    Real droll, dpitch, dyaw;
    Matrix rot_inverse;

    ParticleRotation(Tetrahedron *t, Real droll, Real dpitch, Real dyaw);
    void Apply();
    void Undo();
};

// Cell moves
class CellMove: public Move
{
    public:
    Cell *cell;

    CellMove(Cell *c);
};

class CellShapeMove: public CellMove
{
    public:
    Real dx_max, dy_max, dz_max;
        
    // Last move info for Undo()
    Vector dr_last;
    Real scale;
    int vector_index;

    CellShapeMove(Cell *c, Real dx, Real dy, Real dz);
    void Apply();
    void Undo();
};

#endif
