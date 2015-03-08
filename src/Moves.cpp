
#include "Moves.h"

// Super Move constructor
Move::Move(Real delta_max)
{
    this->delta_max = delta_max;
}
Move::~Move(){}
CellMove::~CellMove(){}
ParticleMove::~ParticleMove(){}

// ParticleMove super class
ParticleMove::ParticleMove(Shape *t, Real delta_max): Move(delta_max)
{
	this->particle = t;
}

// ParticleTranslation class
ParticleTranslation::ParticleTranslation(Shape *t, Real delta_max): ParticleMove(t, delta_max)
{
}

void ParticleTranslation::Apply()
{
    Move::Apply();

    // We remember the details of this move so that it can be undone later if it results in a collision  
    // If dr exists, release that memory before creating a new vector

    Real delta = this->delta_max;

    this->dr = new Vector(u(-delta, delta),
                         u(-delta, delta),
                         u(-delta, delta));

    this->particle->Translate(*(this->dr));
}
void ParticleTranslation::Undo()
{
    Move::Undo();
    Vector dr_reverse(*this->dr);
    dr_reverse *= -1.0;
    this->particle->Translate(dr_reverse);

    delete dr;
}

// ParticleRotation class
ParticleRotation::ParticleRotation(Shape *t, Real delta_max): ParticleMove(t, delta_max)
{
}

void ParticleRotation::Apply()
{
    Move::Apply();

    Real delta = this->delta_max;

    Real roll = u(-delta, delta);
    Real pitch = u(-delta, delta);
    Real yaw = u(-delta, delta);

    Matrix R = Shape::GetRotationMatrix(roll, pitch, yaw);
    this->particle->Rotate(R);
    this->rot_inverse = R.inverse();
}
void ParticleRotation::Undo()
{
    Move::Undo();
    this->particle->Rotate(this->rot_inverse);
}

// CellMove super class
CellMove::CellMove(Cell *c, Real delta_max): Move(delta_max)
{
	this->cell = c;
}

// CellShape class
CellShapeMove::CellShapeMove(Cell *c, Real delta_max): CellMove(c, delta_max)
{
}

void CellShapeMove::Apply()
{
    Move::Apply();

    // Record the old cell tensor in case we need to undo it
    this->h_old = this->cell->h;

    // Change a random element of each basis vector
    Real delta = this->delta_max;

    int i = u(0, 3);
    i=0;

    Vector v = this->cell->h.col(i);
    Matrix R(Shape::GetRotationMatrix(u(-delta, delta), u(-delta,delta), u(-delta,delta)));
    Vector v_rot = R * v;
    this->cell->h.col(i) = v_rot;

//    this->cell->h(i, j) += u(-delta, delta);
//    if(this->cell->h(i, j) < 0.01) this->cell->h(i, j) = 0.01;
}
void CellShapeMove::Undo()
{
    Move::Undo();
    this->cell->h = this->h_old;
}

// CellShape class
CellVolumeMove::CellVolumeMove(Cell *c, Real delta_max): CellMove(c, delta_max)
{
}
void CellVolumeMove::Apply()
{
    Move::Apply();
    // Choose a scale factor
    Real scale = 1 + u(-this->delta_max, this->delta_max);
        
    this->scale = scale;
    this->cell->h *= scale;
}
void CellVolumeMove::Undo()
{
    Move::Undo();
    this->cell->h /= scale;
}
