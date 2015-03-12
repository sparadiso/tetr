#pragma once

#include "Globals.h"
#include "Cell.h"
#include "Shape.h"
#include "Moves.h"

std::ofstream f0("output/dV_0"), f1("output/dV_1");
int pp_counter = 0;
int pg_counter = 0;

template <class ShapeType>
class MCDriver
{
    public:
    Cell cell;
    std::vector<ShapeType*> particles;

    // List of Moves we'll use (separated for convenience since we do cell moves with a different frequency)
    std::vector<ParticleMove*> particle_moves;
    std::vector<CellMove*> cell_moves;

    // Thermodynamic parameters (Pressure and Inverse temperature - the latter only effects the bias potential)
    Real BetaP, Beta;

    // We restrict the minimum internal angles by projecting each basis (unit) vector on the others. 
    // If u_i <dot> u_j > Project_Threshold for any pair i!=j after a cell move, we reject it 
    Real Project_Threshold;

    // Move parameters
    Real p_cell_move; // Probability of choosing a cell move (shape or volume) vs single particle move

    MCDriver(int n_particles, Real p_cell_move = 0.1, Real dr_particle=0.1, Real dtheta_particle=0.2, Real dcell = 0.1);
    ~MCDriver();

    bool CollisionDetectedWith(ShapeType *t);
    void UpdatePeriodicImages(ShapeType *shape);
    void InitializePeriodicImages(ShapeType *shape);
    Real GetPackingFraction();
    bool MakeMove();

    // Setters to modify the maximum move size for cell shape/particle moves
    void SetCellShapeDelta(Real delta);
    void SetParticleTranslationDelta(Real delta);

    std::string ToString(bool PrintImagesYN=false);

    // Simple controller to change the max move sizes to achieve a target acceptance rate.
    // Anecdotally, this doesn't seem to help too much.
    void UpdateMoveSizes(Real TargetAcceptance=0.3);
};

template <class ShapeType>
MCDriver<ShapeType>::MCDriver(int n_particles, Real p_cell_move,
                              Real dr, Real dtheta_particle,  // Particle Move Parameters
                              Real dtheta_cell): cell(n_particles)
{
    this->p_cell_move = p_cell_move;

    // Populate the cell moves
    this->cell_moves.push_back(new CellShapeMove(&this->cell, dtheta_cell));

    // Initialize the particles in valid (non-overlapping) positions
    for(int i=0;i<n_particles;i++)
    {
        // Random starting orientation
        Real roll = (rand() % 1000) / 1000.0 * 2*PI;
        Real pitch = (rand() % 1000) / 1000.0 * 2*PI;
        Real yaw = (rand() % 1000) / 1000.0 * 2*PI;
        
        // Initialize particle at the center, then try translation moves until one returns non-colliding 
        Vector v(.5*n_particles*u(0, .1), .5*n_particles*u(0, .1), .5*n_particles*u(0, .1));

        // Create the particle and push it to the driver's particle list
        ShapeType *t = new ShapeType(v);
        t->Rotate(roll, pitch, yaw);
        this->particles.push_back(t);

        // Add a translation/rotation move for this particle
        this->particle_moves.push_back(new ParticleTranslation(t, dr));

        // Keep applying particle translations until a valid position is found
        this->InitializePeriodicImages(t);
        while( this->CollisionDetectedWith(t) )
        {
            this->particle_moves.back()->Apply();
            this->cell.WrapShape(t);
            this->UpdatePeriodicImages(t);
        }
           
        // Now that we've settled on a position, wrap it and update the periodic images
        this->cell.WrapShape(t);
        this->UpdatePeriodicImages(t);
        
        // Finally, add a rotation move
        this->particle_moves.push_back(new ParticleRotation(t, dtheta_particle));
    }

    // Create the cell and give it references to the particles
    for(int i=0;i<n_particles;i++)
        cell.particles.push_back(particles[i]);
}

// Destructor
template <class ShapeType>
MCDriver<ShapeType>::~MCDriver()
{
    for(uint i=0;i<this->particles.size();i++)
        delete this->particles[i];

    for(uint i=0;i<this->particle_moves.size();i++)
        delete this->particle_moves[i];

    for(uint i=0;i<this->cell_moves.size();i++)
        delete this->cell_moves[i];
}

template <class ShapeType>
void MCDriver<ShapeType>::SetParticleTranslationDelta(Real delta)
{
    for(uint i=0;i<this->particle_moves.size();i+=2) 
    {
        this->particle_moves[i]->delta_max = delta;
    }
}

template <class ShapeType>
void MCDriver<ShapeType>::SetCellShapeDelta(Real delta)
{
    if(this->cell_moves.size() > 0)
        this->cell_moves[0]->delta_max = delta;
}

// Returns `true` if collisions are detected
template <class ShapeType>
bool MCDriver<ShapeType>::CollisionDetectedWith(ShapeType *t)
{
    // First, check collisions with other particles
    for(uint i=0;i<particles.size();i++)
        if (t->Intersects(particles[i]) && (t->GetCOM() - particles[i]->GetCOM()).norm() > .00001 )
        {
            return true;
        }

    for(uint i=0;i<particles.size();i++)
    {
        ShapeType *particle = this->particles[i];

        // Now check collisions with periodic images
        for(uint j=0;j<particle->periodic_images.size();j++)
            if (t->Intersects(particle->periodic_images[j]))
                return true;
    }

    return false;
}

template <class ShapeType>
bool MCDriver<ShapeType>::MakeMove()
{
    // Assume the move is good until proven otherwise
    bool accepted = true;

    Move *AttemptedMove;
        
    // First, choose the move to make
    if (u(0, 1) < this->p_cell_move)
    {
        //std::cout << "Trying cell move" << std::endl;

        // Make a CellMove (shape or volume change)
        int move_index = u(0,1) * this->cell_moves.size();
        CellMove *move = this->cell_moves[move_index];
        AttemptedMove = move;

        // Apply the move and record the change in cell volume
        Real V_Before = this->cell.GetVolume();
        move->Apply();
        Real V_After = this->cell.GetVolume();
        //std::cout << "Attempting DV="<<(V_After-V_Before) << std::endl;
    
        // Print out the dV for this move
        if(move_index==0)
            f0 << V_After - V_Before << std::endl;
        else
            f1 << V_After - V_Before << std::endl;
        f0.flush();
        f1.flush();

        // Enforce a minimum cell vector length
        for(int i=0;i<3;i++)
            if (this->cell.h.col(i).norm() < .5)
                accepted = false;
        

        // Check the interior angles
//        Real Project_Threshold = 0.2;
        Vector e0(this->cell.h.col(0)); e0 /= e0.norm();
        Vector e1(this->cell.h.col(1)); e1 /= e1.norm();
        Vector e2(this->cell.h.col(2)); e2 /= e2.norm();

        Real overlap_0 = std::abs(e0.dot(e1));
        Real overlap_1 = std::abs(e0.dot(e2));
        Real overlap_2 = std::abs(e1.dot(e2));
    
        // The last case is a flat parallelpiped (after strong shear)
        // Avoid this by checking the overlap of the unit normal of each plane and the last basis vector
        Vector u_plane;
        u_plane = e0.cross(e1); u_plane /= u_plane.norm(); if(std::abs(e2.dot(u_plane)) < 1-Project_Threshold) accepted = false;
        u_plane = e0.cross(e2); u_plane /= u_plane.norm(); if(std::abs(e1.dot(u_plane)) < 1-Project_Threshold) accepted = false;
        u_plane = e1.cross(e2); u_plane /= u_plane.norm(); if(std::abs(e0.dot(u_plane)) < 1-Project_Threshold) accepted = false;

        if( 
            (overlap_0 > Project_Threshold) || 
            (overlap_1 > Project_Threshold) || 
            (overlap_2 > Project_Threshold)
          )
        {
            accepted = false;
        }

        // If we haven't ruled it out based on interior angles, check for collisions after the volume change
        if(accepted)
        {
            // Have to update the images before collision detection 
            for(uint i=0;i<this->particles.size();i++)
                this->UpdatePeriodicImages(this->particles[i]);

            for(uint i=0;i<this->particles.size();i++)
            {
                // Check for collisions 
                accepted = !(this->CollisionDetectedWith(this->particles[i]));

                if(!accepted)
                    break;
            }
        }
        
        // If accepted is still true, then no collisions happened. In this case, accept on the Boltzmann factor
        if(accepted)
        {
            //std::cout << "Checking boltzmann P="<<this->BetaP<< ", factor=" << exp(-this->BetaP*(V_After-V_Before)) << std::endl;
            if (u(0,1) < exp(-this->BetaP*(V_After-V_Before)))
            {
                accepted = true;

                for(uint i=0;i<this->particles.size();i++)          
                    this->cell.WrapShape(this->particles[i]);
            }
            else
                accepted = false;

        }

        // If the move wasn't accepted, we need to revert the cell 
        if(!accepted)
        {
            AttemptedMove->Undo();
            for(uint i=0;i<this->particles.size();i++)
                this->UpdatePeriodicImages(this->particles[i]);
        }
    }
    else
    {
        // randomly select a particle move
        int move_index = rand() % this->particle_moves.size();
        ParticleMove *move = this->particle_moves[move_index];
        AttemptedMove = move;

        ShapeType *t = reinterpret_cast<ShapeType*>(move->particle);

        Vector s_old = this->cell.PartialCoords(t->GetCOM());
        move->Apply();
        Vector s_new = this->cell.PartialCoords(t->GetCOM());

        // Check for collisions - CollisionDetectedWith returns true if collisions are detected
        accepted = !(this->CollisionDetectedWith(t));

        // If no collisions, check on internal energy due to the potential
        if(accepted && this->Beta > 0)
        {
            // Parabolic potential 
            Vector center(.5,.5,.5);
            Real E_New = (s_new-center).norm();
            Real E_Old = (s_old-center).norm();

            Real p = exp(-this->Beta*(E_New-E_Old));
            if (u(0,1) < p)
            {
                accepted = true;
            }
            else
                accepted = false;
        }
    
        if(accepted)
        {
            this->cell.WrapShape(t);
            this->UpdatePeriodicImages(t);
        }

        if(!accepted)
            AttemptedMove->Undo();
    }

    return accepted;
}

template <class ShapeType>
Real MCDriver<ShapeType>::GetPackingFraction()
{
    return this->particles.size() * this->particles[0]->GetVolume() / this->cell.GetVolume();
}

// Update periodic images
template <class ShapeType>
void MCDriver<ShapeType>::UpdatePeriodicImages(ShapeType *t)
{
    int ghost_idx = 0;

    // [j, k, l] defines a periodic image to check - we only check the first shell, which is *not* rigorous
    // but we also restrict the minimum internal cell angles to avoid risking second-shell collisions.
    for(int j=-1;j<2;j++)
    for(int k=-1;k<2;k++)
    for(int l=-1;l<2;l++)
    {
        if(!(j==0&&k==0&&l==0))
        {
            Vector index(j,k,l);

            ShapeType *ghost = reinterpret_cast<ShapeType*>(t->periodic_images[ghost_idx++]);
            // Copy the particle
            for(uint i=0;i<t->vertices.size();i++)
                ghost->vertices[i] = t->vertices[i];

            // Translate the ghost to its image position
            ghost->Translate(this->cell.h * index);
        }
    }
}

// Generate all periodic images of the particles in the cell
template <class ShapeType>
void MCDriver<ShapeType>::InitializePeriodicImages(ShapeType* t)
{
    // Clear the current list of images if it's already populated
    for(uint i=0;i<t->periodic_images.size();i++)
        delete t->periodic_images[i];
    t->periodic_images.clear();

    for(int j=-1;j<2;j++)
    for(int k=-1;k<2;k++)
    for(int l=-1;l<2;l++)
    {
        if(!(j==0&&k==0&&l==0))
        {
            t->periodic_images.push_back(new ShapeType(*t));
        }
    }

    this->UpdatePeriodicImages(t);
}

template <class ShapeType>
std::string MCDriver<ShapeType>::ToString(bool PrintImages)
{
    std::string s = "";

    // First, the cell tensor
    s += this->cell.ToString();
    s += "\n";
    
    // Now the particles
    for(uint i=0;i<this->particles.size();i++)
    {
        s += std::string("particle: ") + this->particles[i]->ToString() + "\n";
    }

    if(PrintImages)
    {
        // Now the ghosts
        for(uint i=0;i<this->particles.size();i++)
        {
            ShapeType *shape = this->particles[i];
            for(uint j=0;j<shape->periodic_images.size();j++)
            {
                s += std::string("ghost: ") + shape->periodic_images[j]->ToString() + "\n";
            }
        }
    }

    return s;
}

template <class ShapeType>
void MCDriver<ShapeType>::UpdateMoveSizes(Real TargetAcceptance)
{
    // For each move, compute its acceptance rate and modify its max_delta accordingly
    for(uint i=0;i<this->cell_moves.size();i++)
    {
        Move *m = this->cell_moves[i];
        Real ratio = m->GetRatio();

        if (ratio > TargetAcceptance)
            m->delta_max *= 2;
        else
            m->delta_max /= 2;

        m->Reset();
    }
}
