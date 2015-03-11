
#include "Moves.h"

// Super Move constructor
Move::Move(Real delta_max)
{
    accepted_moves = 0;
    total_moves = 0;
    this->delta_max = delta_max;

}
Move::~Move(){}
CellMove::~CellMove(){}
ParticleMove::~ParticleMove(){}

// ParticleMove super class
ParticleMove::ParticleMove(Shape *t, Real delta_max): Move(delta_max)
{
	this->particle = t;
    for(uint i=0;i<this->particle->vertices.size();i++)
        this->vertices_old.push_back(Vector(t->vertices[i]));
}

// ParticleTranslation class
ParticleTranslation::ParticleTranslation(Shape *t, Real delta_max): ParticleMove(t, delta_max)
{
}

void ParticleTranslation::Apply()
{
    Move::Apply();

    for(uint i=0;i<this->particle->vertices.size();i++)
        this->vertices_old[i] = this->particle->vertices[i];

    // We remember the details of this move so that it can be undone later if it results in a collision  
    // If dr exists, release that memory before creating a new vector

    Real delta = this->delta_max;

    Vector dr(u(-delta, delta),
             u(-delta, delta),
             u(-delta, delta));

    this->particle->Translate(dr);
}

// ParticleRotation class
ParticleRotation::ParticleRotation(Shape *t, Real delta_max): ParticleMove(t, delta_max)
{
}

void ParticleRotation::Apply()
{
    Move::Apply();

    for(uint i=0;i<this->particle->vertices.size();i++)
        this->vertices_old[i] = this->particle->vertices[i];

    Real delta = this->delta_max;

    Real roll = u(-delta, delta);
    Real pitch = u(-delta, delta);
    Real yaw = u(-delta, delta);

    Matrix R = Shape::GetRotationMatrix(roll, pitch, yaw);
    this->particle->Rotate(R);
}
void ParticleMove::Undo()
{
    Move::Undo();
    for(uint i=0;i<this->particle->vertices.size();i++)
        this->particle->vertices[i] = this->vertices_old[i];
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

    // Generate a random strain tensor 
    Real delta = this->delta_max;

    Matrix e;
    e << u(-delta,delta), u(-delta,delta), u(-delta,delta),
         u(-delta,delta), u(-delta,delta), u(-delta,delta),
         u(-delta,delta), u(-delta,delta), u(-delta,delta);

    // Make strain tensor symmetric
    e(0,1) = e(1,0);
    e(0,2) = e(2,0);
    e(1,2) = e(2,1);

    // Record the strain tensor so we can undo this move after
    Matrix cell_update = Matrix::Identity() + e;
    this->h_old = this->cell->h;

    // We move the particles along with the cell tensor to aid compression
    std::vector<Vector> s_com;
    for(uint i=0;i<this->cell->particles.size();i++)   
        s_com.push_back(this->cell->PartialCoords(this->cell->particles[i]->GetCOM()));

    // Apply the strain tensor to update the cell
    this->cell->h *= cell_update;

    // Now apply the appropriate translations to each particle
    for(uint i=0;i<this->cell->particles.size();i++)   
    {
        Shape *p = this->cell->particles[i];
        Vector r_com(p->GetCOM());
        Vector r_com_new(this->cell->h * s_com[i]);
        p->Translate(r_com_new - r_com);
    }
}
void CellMove::Undo()
{
    Move::Undo();

    // Copy Pasta from Apply() above
    // We move the particles along with the cell tensor to aid compression
    std::vector<Vector> s_com;
    for(uint i=0;i<this->cell->particles.size();i++)   
        s_com.push_back(this->cell->PartialCoords(this->cell->particles[i]->GetCOM()));

    // Apply the strain tensor to update the cell
    this->cell->h = this->h_old;

    // Now apply the appropriate translations to each particle
    for(uint i=0;i<this->cell->particles.size();i++)   
    {
        Shape *p = this->cell->particles[i];
        Vector r_com(p->GetCOM());
        Vector r_com_new(this->cell->h * s_com[i]);
        p->Translate(r_com_new - r_com);
    }
}

// CellShape class
CellVolumeMove::CellVolumeMove(Cell *c, Real delta_max): CellMove(c, delta_max)
{
}
void CellVolumeMove::Apply()
{
    Move::Apply();

    // Choose a scale factor
    Real dv = u(-this->delta_max, this->delta_max);

    int b0 = u(0,3);
    int b1 = (b0 + 1) % 3; Vector e1 = this->cell->h.col(b1);
    int b2 = (b0 + 2) % 3; Vector e2 = this->cell->h.col(b2);
        
    Vector c = e1.cross(e2);
    Real area = c.norm();
    Vector dr = this->cell->h.col(b0); dr /= dr.norm(); dr *= dv/area;

    // Add length to the vector so that area*dl = dv
    this->cell->h.col(b0) += dr;
}
