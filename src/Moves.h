#pragma once

#include "Globals.h"
#include "Cell.h"
#include "Shape.h"

// These classes help simplify the logic of applying/undoing Monte Carlo moves. Generally speaking, this is probably overkill.
class Move
{
    public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Real delta_max;
    int accepted_moves, total_moves;

    // Constructor/Destructor
    Move(Real delta_max);
    virtual ~Move();

    // Virtual methods to be overloaded for specific move types
    virtual void Apply() {accepted_moves++;total_moves++;}
    virtual void Undo() {accepted_moves--;}

    // Some reporting functions for acceptance rates
    Real GetRatio(){return 1.*accepted_moves / total_moves;}
    void Reset(){accepted_moves = 0; total_moves = 0;}
};

// Particle moves
class ParticleMove: public Move
{
    public:
    Shape *particle;
    std::vector<Vector> vertices_old;

    // Constructor/Destructor
    ParticleMove(Shape *t, Real delta_max);

    // All particle moves share a common Undo: vertices are reset to `vertices_old`
    void Undo();
};

class ParticleTranslation: public ParticleMove
{
    public:
        
    // Constructor
    ParticleTranslation(Shape *t, Real delta_max);

    // Translate the particle by a random displacement vector in R^3
    void Apply();
};

class ParticleRotation: public ParticleMove
{
    public:

    // Constructor
    ParticleRotation(Shape *t, Real delta_max);

    // Rotate the particle by a set of 3 random angles (around the 3 principle (intrinsic) axes in R^3 (e_x, e_y, e_z))
    void Apply();
};

// Cell moves
class CellMove: public Move
{
    public:
    Cell *cell;
    Matrix h_old;

    // Constructor/Destructor
    CellMove(Cell *c, Real delta_max);

    void Undo();
};

class CellShapeMove: public CellMove
{
    public:
    
    // Constructor
    CellShapeMove(Cell *c, Real delta_max);

    // Cell shape is updated by h_new = (I + e) * h where e is a symmetric strain tensor with small elements drawn from {-delta_max, delta_max}
    void Apply();
};

