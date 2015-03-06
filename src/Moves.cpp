
#include "Moves.h"

// ParticleMove super class
ParticleMove::ParticleMove(Tetrahedron *t)
{
	this->particle = t;
}

// ParticleTranslation class
ParticleTranslation::ParticleTranslation(Tetrahedron *t, Real dr_max): ParticleMove(t)
{
    this->dr_max = dr_max;
}

void ParticleTranslation::Apply()
{
    // We remember the details of this move so that it can be undone later if it results in a collision  
    // If dr exists, release that memory before creating a new vector
    try { delete dr; } catch (int exception) {}

    this->dr = new Vector(u(-this->dr_max, this->dr_max),
                         u(-this->dr_max, this->dr_max),
                         u(-this->dr_max, this->dr_max));

    this->particle->Translate(this->dr);
}
void ParticleTranslation::Undo()
{
    Vector dr_reverse(*this->dr);
    dr_reverse *= -1.0;
    this->particle->Translate(dr_reverse);
}

// ParticleRotation class
ParticleRotation::ParticleRotation(Tetrahedron *t, Real droll, Real dpitch, Real dyaw): ParticleMove(t)
{
    this->droll = droll;
    this->dpitch = dpitch;
    this->dyaw = dyaw;
}

void ParticleRotation::Apply()
{
    Real roll = u(-this->droll, this->droll);
    Real pitch = u(-this->dpitch, this->dpitch);
    Real yaw = u(-this->dyaw, this->dyaw);

    this->rot_inverse = this->particle->Rotate(roll, pitch, yaw).inverse();
}
void ParticleRotation::Undo()
{
    this->particle->Rotate(this->rot_inverse);
}

// CellMove super class
CellMove::CellMove(Cell *c)
{
	this->cell = c;
}

// ParticleTranslation class
CellShapeMove::CellShapeMove(Cell *c, Real dx, Real dy, Real dz): CellMove(c)
{
    this->dx_max = dx;
    this->dy_max = dy;
    this->dz_max = dz;
}
void CellShapeMove::Apply()
{
    // Rotate a random basis vector
    this->vector_index = rand() % 3;
    Vector v(this->cell->h.col(this->vector_index));

    // Get original length
    Real l0 = v.norm();
    Real dx_last = u(-this->dx_max, this->dx_max);    
    Real dy_last = u(-this->dy_max, this->dy_max);    
    Real dz_last = u(-this->dz_max, this->dz_max);    

    Vector dr(dx_last, dy_last, dz_last);
    this->dr_last = dr;
    
    // After modifying the vector, we need to rescale to its original length
    this->cell->h.col(this->vector_index) += dr;
    this->scale = l0 / this->cell->h.col(this->vector_index).norm();
    this->cell->h.col(this->vector_index) *= scale;
}
void CellShapeMove::Undo()
{
    this->cell->h.col(this->vector_index) /= scale;
    this->cell->h.col(this->vector_index) -= this->dr_last;    
}
