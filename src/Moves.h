#pragma once

#include "Globals.h"
#include "Cell.h"
#include "Shape.h"

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
    virtual ~Move();
};

// Particle moves
class ParticleMove: public Move
{
    public:
    Shape *particle;

    ParticleMove(Shape *t, Real delta_max);

    virtual ~ParticleMove();
};

class ParticleTranslation: public ParticleMove
{
    public:
    Vector *dr;

    ParticleTranslation(Shape *t, Real delta_max);
    void Apply();
    void Undo();
};

class ParticleRotation: public ParticleMove
{
    public:
    Matrix rot_inverse;

    ParticleRotation(Shape *t, Real delta_max);
    void Apply();
    void Undo();
};

// Cell moves
class CellMove: public Move
{
    public:
    Cell *cell;

    CellMove(Cell *c, Real delta_max);
    virtual ~CellMove();
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

