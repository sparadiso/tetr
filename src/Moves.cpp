
#include "Moves.h"

// Super Move constructor
Move::Move(Real delta_max)
{
    this->delta_max = delta_max;
}

// ParticleMove super class
ParticleMove::ParticleMove(Tetrahedron *t, Real delta_max): Move(delta_max)
{
	this->particle = t;
}

// ParticleTranslation class
ParticleTranslation::ParticleTranslation(Tetrahedron *t, Real delta_max): ParticleMove(t, delta_max)
{
}

void ParticleTranslation::Apply()
{
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
    Vector dr_reverse(*this->dr);
    dr_reverse *= -1.0;
    this->particle->Translate(dr_reverse);

    delete dr;
}

// ParticleRotation class
ParticleRotation::ParticleRotation(Tetrahedron *t, Real delta_max): ParticleMove(t, delta_max)
{
}

void ParticleRotation::Apply()
{
    Real delta = this->delta_max;

    Real roll = u(-delta, delta);
    Real pitch = u(-delta, delta);
    Real yaw = u(-delta, delta);

    this->rot_inverse = this->particle->Rotate(roll, pitch, yaw).inverse();
}
void ParticleRotation::Undo()
{
    this->particle->Rotate(this->rot_inverse);
}

// CellMove super class
CellMove::CellMove(Cell *c, Real delta_max): Move(delta_max)
{
	this->cell = c;
}

// ParticleTranslation class
CellShapeMove::CellShapeMove(Cell *c, Real delta_max): CellMove(c, delta_max)
{
}

void CellShapeMove::Apply()
{
    // Record the old cell tensor in case we need to undo it
    this->h_old = this->cell->h;

    // Change a random element of each basis vector
    Real delta = this->delta_max;

    int i = u(0, 3);
    int j = u(0,3);
    this->cell->h(i, i) += u(-delta, delta);
    //this->cell->h(0,0) -= 0.05;
    
//        if (this->cell->h(i, j) < 0.5) 
//            this->cell->h(i, j) = 0.5;
}
void CellShapeMove::Undo()
{
    this->cell->h = this->h_old;
}
