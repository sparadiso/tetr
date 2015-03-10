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
    int accepted_moves, total_moves;

    Move(Real delta_max);
    virtual void Apply() {accepted_moves++;total_moves++;}
    virtual void Undo() {accepted_moves--;}
    Real GetRatio(){return 1.*accepted_moves / total_moves;}
    void Reset(){accepted_moves = 0; total_moves = 0;}
    virtual ~Move();
};

// Particle moves
class ParticleMove: public Move
{
    public:
    Shape *particle;
    std::vector<Vector> vertices_old;

    ParticleMove(Shape *t, Real delta_max);

    virtual ~ParticleMove();
    void Undo();
};

class ParticleTranslation: public ParticleMove
{
    public:

    ParticleTranslation(Shape *t, Real delta_max);
    void Apply();
};

class ParticleRotation: public ParticleMove
{
    public:

    ParticleRotation(Shape *t, Real delta_max);
    void Apply();
};

// Cell moves
class CellMove: public Move
{
    public:
    Cell *cell;
    Matrix cell_update;

    CellMove(Cell *c, Real delta_max);
    virtual ~CellMove();
    void Undo();
};

class CellShapeMove: public CellMove
{
    public:

    CellShapeMove(Cell *c, Real delta_max);
    void Apply();
};

class CellVolumeMove: public CellMove
{
    public:
        
    CellVolumeMove(Cell *c, Real delta_max);
    void Apply();
};

