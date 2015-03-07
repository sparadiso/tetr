#pragma once

#include "Globals.h"
#include "Cell.h"
#include "Shape.h"
#include "Moves.h"

template <class ShapeType>
class MCDriver
{
    public:

    Cell cell;
    std::vector<ShapeType*> particles;

    // List of Moves we'll use (particle vs cell)
    std::vector<ParticleMove*> particle_moves;
    std::vector<CellMove*> cell_moves;

    // Thermo parameters
    Real BetaP, Beta;

    // Move parameters
    Real p_cell_move; // Probability of choosing a cell move (shape or volume) vs single particle move

    MCDriver(int n_particles, Real p_cell_move = 0.1, Real dr_particle=0.1, Real dtheta_particle=0.2, Real dcell = 0.02);

    ~MCDriver();

    static void __FreeGhosts(std::vector<ShapeType*> ghosts);

    bool CheckCollisionsWith(ShapeType *t, std::vector<ShapeType*> ghosts);
    std::vector<ShapeType*> GetPeriodicGhosts();
    void MakeMove();
    void SetCellShapeDelta(Real delta);
    std::string ToString();
};

template <class ShapeType>
MCDriver<ShapeType>::MCDriver(int n_particles, Real p_cell_move,
                Real dr, Real dtheta_particle,  // Particle Move Parameters
                Real dtheta_cell) : cell(n_particles)      // Cell Move Parameters
{
    this->p_cell_move = p_cell_move;

    // Populate the cell moves
    this->cell_moves.push_back(new CellShapeMove(&this->cell, dtheta_cell));

    // Initialize the particles in valid (non-overlapping) positions
    for(int i=0;i<n_particles;i++)
    {
        Real roll = (rand() % 1000) / 1000.0 * 2*PI;
        Real pitch = (rand() % 1000) / 1000.0 * 2*PI;
        Real yaw = (rand() % 1000) / 1000.0 * 2*PI;
        
        // Initialize particle at the center, then try translation moves until one sticks
        Vector v(.5*n_particles, .5*n_particles, .5*n_particles);

        ShapeType *t = new ShapeType(v);
        t->Rotate(roll, pitch, yaw);
        this->particles.push_back(t);

        // Add a translation/rotation move for this particle
        this->particle_moves.push_back(new ParticleTranslation(t, dr));

        this->particle_moves.back()->Apply();

        // Keep applying particle translations until a valid position is found
        while( this->CheckCollisionsWith(t, this->GetPeriodicGhosts()) )
        {
            this->particle_moves.back()->Apply();
        }
           
        // Populate the particle moves
        this->particle_moves.push_back(new ParticleRotation(t, dtheta_particle));
    }
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
void MCDriver<ShapeType>::SetCellShapeDelta(Real delta)
{
    this->cell_moves[0]->delta_max = delta;
}

template <class ShapeType>
void MCDriver<ShapeType>::__FreeGhosts(std::vector<ShapeType*> ghosts)
{
    for (uint j=0;j<ghosts.size();j++) 
        delete ghosts[j];
}

// Returns TRUE if collisions ARE detected
template <class ShapeType>
bool MCDriver<ShapeType>::CheckCollisionsWith(ShapeType *t, std::vector<ShapeType*> ghosts)
{

    // First, check collisions with other particles
    for(uint i=0;i<particles.size();i++)
        if (t->Intersects(particles[i]) && (t->GetCOM() - particles[i]->GetCOM()).norm() > 1E-5 )
        {
//            std::cout << "Collided with other particle" << std::endl;
            return true;
        }

    // Now check collisions with periodic images
    for(uint i=0;i<ghosts.size();i++)
        if (t->Intersects(ghosts[i]))
        {
//            std::cout << "Collided with image particle" << std::endl;
            return true;
        }

    return false;
}

template <class ShapeType>
void MCDriver<ShapeType>::MakeMove()
{
    // Assume the move is good until proven otherwise
    bool accepted = true;

    Move *AttemptedMove;
        
    // First, choose the move to make
    if (u(0, 1) < this->p_cell_move)
    {
        // Make a CellMove (shape or volume change)
        int move_index = rand() % this->cell_moves.size();
        CellMove *move = this->cell_moves[move_index];
        AttemptedMove = move;

        // Apply the move and record the change in cell volume
        Real V_Before = this->cell.GetVolume();
        move->Apply();
        Real V_After = this->cell.GetVolume();

//        std::cout << V_Before << std::endl;
//        std::cout << V_After << std::endl;

        // Compute ghost images if this is a volume move
        std::vector<ShapeType*> ghosts = this->GetPeriodicGhosts();

        for(uint i=0;i<this->particles.size();i++)
        {
            // Check for collisions - CheckCollisionsWith returns true if collisions are detected
            accepted = !(this->CheckCollisionsWith(this->particles[i], ghosts));
            if(!accepted)
                break;
        }
        
        // If accepted is still true, then no collisions happened. In this case, accept on the Boltzmann factor
        if(accepted)
        {
            if (u(0,1) < exp(-this->BetaP*(V_After-V_Before)))
                accepted = true;
            else
                accepted = false;
        }

//        std::cout << "Accepted: " << accepted << std::endl;

        // Free up the ghost images
        __FreeGhosts(ghosts);
    }
    else
    {
        // randomly select a particle move
        int move_index = rand() % this->particle_moves.size();
        ParticleMove *move = this->particle_moves[move_index];
        AttemptedMove = move;

        ShapeType *t = reinterpret_cast<ShapeType*>(move->particle);
        this->cell.WrapShape(t);

        Vector s_old = this->cell.PartialCoords(t->GetCOM());
        //std::cout << s_old << std::endl;
        move->Apply();
        Vector s_new = this->cell.PartialCoords(t->GetCOM());

        // Compute ghost images 
        std::vector<ShapeType*> ghosts = this->GetPeriodicGhosts();
        
        // Check for collisions - CheckCollisionsWith returns true if collisions are detected
        accepted = !(this->CheckCollisionsWith(t, ghosts));

        // If no collisions, check on internal energy due to the potential
        if(accepted)
        {
            // Parabolic potential 
            Vector center(.5,.5,.5);
            Real E_New = (s_new-center).norm();
            Real E_Old = (s_old-center).norm();

            Real p = exp(-this->Beta*(E_New-E_Old));
            if (u(0,1) < p)
                accepted = true;
            else
                accepted = false;

//            std::cout << "Final acceptance: " << accepted << std::endl;
        }

        __FreeGhosts(ghosts);
    }

    if(!accepted)
    {
//        std::cout << "Not accepted.." << accepted << std::endl;
        AttemptedMove->Undo();
    }
}

// Generate all periodic images of the particles in the cell
template <class ShapeType>
std::vector<ShapeType*> MCDriver<ShapeType>::GetPeriodicGhosts()
{
    Eigen::MatrixXd all_translations(26, 3);
    all_translations <<  0, 0, 0,
                        0, 0, 1,
                        0, 0, 2,
                        0, 1, 0,
                        0, 1, 1,
                        0, 1, 2,
                        0, 2, 0,
                        0, 2, 1,
                        0, 2, 2,
                        1, 0, 0,
                        1, 0, 1,
                        1, 0, 2,
                        1, 1, 0,
                        1, 1, 2,
                        1, 2, 0,
                        1, 2, 1,
                        1, 2, 2,
                        2, 0, 0,
                        2, 0, 1,
                        2, 0, 2,
                        2, 1, 0,
                        2, 1, 1,
                        2, 1, 2,
                        2, 2, 0,
                        2, 2, 1,
                        2, 2, 2;

    all_translations.array() -= 1.;

    Eigen::MatrixXd displacement_vectors = all_translations * this->cell.h;

    std::vector<ShapeType*> ghosts;

    for(uint i=0;i<particles.size();i++)
    {
        ShapeType *t = particles[i];
        Vector com = t->GetCOM();

        for(int j=0;j<26;j++)
        {
            Vector v(displacement_vectors.row(j));
            if((com-v).norm() < 1. || true)
            {
                ShapeType *ghost = new ShapeType(*t);
                ghost->Translate(v);
                ghosts.push_back(ghost);
            }
        }

        /*
        std::vector<Vector> displacements;

        for(uint j=0;j<t->vertices.size();j++)
        {
            Vector v = this->cell.PeriodicImage(t->vertices[j]);            
            Vector dv = v - t->vertices[j];

            bool repeat = false;

            if ( dv.norm() < 1E-5)
                repeat = true;

            // Check to see if this displacement exists already 
            for(uint k=0;k<displacements.size();k++)
            {
                if(repeat)
                    break;

                dv = v - displacements[k];

//                std::cout << "v: \n"<<v << std::endl;
//                std::cout << "dis: \n"<< dv << std::endl;
//                std::cout << "norm: \n"<< dv.norm() << std::endl;

                if(dv.norm() < 1E-5)
                {
                    repeat = true;
                }
            }
            if (!repeat)
            {  
                displacements.push_back(v - t->vertices[j]);
            }
        }

        // FOR EACH DISPLACEMENT, CREATE A GHOST BY COPYING/TRANSLATING `t` AND ADD IT TO THE LIST
        for(uint j=0;j<displacements.size();j++)
        {
            ShapeType *ghost = new ShapeType(*t);
//            std::cout << "Ghost Before: " << ghost->ToString() << std::endl;
            ghost->Translate(displacements[j]);
//            std::cout << "Ghost After: " << ghost->ToString() << std::endl;
            ghosts.push_back(ghost);
        }
        */
    }

//    for(uint i=0;i<ghosts.size();i++)
//        std::cout << "Ghosts Returned: " << ghosts[i]->ToString() << std::endl;

    return ghosts;
}

template <class ShapeType>
std::string MCDriver<ShapeType>::ToString()
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

    // Now the ghosts
    std::vector<ShapeType*> ghosts = this->GetPeriodicGhosts();
    for(uint i=0;i<ghosts.size();i++)
    {
        s += std::string("ghost: ") + ghosts[i]->ToString() + "\n";
    }
    __FreeGhosts(ghosts);

    return s;
}
