#include "Globals.h"
#include "MCDriver.h"

MCDriver::MCDriver(int n_particles, Real p_cell_move,
                Real dr, Real dtheta_particle, // Particle Move Parameters
                Real dtheta_cell, Real dv) : cell() // Cell Move Parameters
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
        
        // Initialize particle at the origin, then try translation moves until one sticks
        Vector v(0, 0, 0);

        Tetrahedron *t = new Tetrahedron(v, roll, pitch, yaw);
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

void __FreeGhosts(std::vector<Tetrahedron*> ghosts)
{
    for (uint j=0;j<ghosts.size();j++) 
        delete ghosts[j];
}

// Returns TRUE if collisions ARE detected
bool MCDriver::CheckCollisionsWith(Tetrahedron *t, std::vector<Tetrahedron*> ghosts)
{
    // First, check collisions with other particles
    for(uint i=0;i<particles.size();i++)
        if (t->Intersects(particles[i]) && (t->GetCOM() - particles[i]->GetCOM()).norm() > 1E-5 )
        {
            std::cout << "Collided with other particle" << std::endl;
            return true;
        }

    // Now check collisions with periodic images
    for(uint i=0;i<ghosts.size();i++)
        if (t->Intersects(ghosts[i]))
        {
            std::cout << "Collided with image particle" << std::endl;
            return true;
        }

    return false;
}

using namespace std;

void MCDriver::MakeMove()
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

        // Compute ghost images if this is a volume move
        std::vector<Tetrahedron*> ghosts = this->GetPeriodicGhosts();

        for(uint i=0;i<this->particles.size();i++)
        {
            // Check for collisions - CheckCollisionsWith returns true if collisions are detected
            accepted = !(this->CheckCollisionsWith(this->particles[i], ghosts));
        }
        
        // If accepted is still true, then no collisions happened. In this case, accept on the Boltzmann factor
        if(accepted)
        {
            accepted = u(0,1) < exp(-this->BetaP*(V_After-V_Before));
        }

        // Free up the ghost images
        __FreeGhosts(ghosts);
    }
    else
    {
        // randomly select a particle move
        int move_index = rand() % this->particle_moves.size();
        ParticleMove *move = this->particle_moves[move_index];
        AttemptedMove = move;

        move->Apply();

        // Compute ghost images 
        std::vector<Tetrahedron*> ghosts = this->GetPeriodicGhosts();
        
        // Check for collisions - CheckCollisionsWith returns true if collisions are detected
        accepted = !(this->CheckCollisionsWith(move->particle, ghosts));

        __FreeGhosts(ghosts);
    }

    if(!accepted)
    {
        AttemptedMove->Undo();
    }
}

// Generate all periodic images of the particles in the cell
std::vector<Tetrahedron*> MCDriver::GetPeriodicGhosts()
{
    std::vector<Tetrahedron*> ghosts;
    for(uint i=0;i<particles.size();i++)
    {
        Tetrahedron *t = particles[i];
        std::vector<Vector> displacements;

        for(uint j=0;j<t->vertices.size();j++)
        {
            Vector v = this->cell.PeriodicImage(t->vertices[j]);            
            Vector dv = v - t->vertices[j];

//            std::cout << "Vertex: \n" << t->vertices[j] << std::endl;
//            std::cout << "Image: \n" << v << std::endl;

//            for(uint k=0;k<displacements.size();k++)
//                std::cout << "Displacement: \n" << displacements[k] << std::endl;
            
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
            Tetrahedron *ghost = new Tetrahedron(*t);
//            std::cout << "Ghost Before: " << ghost->ToString() << std::endl;
            ghost->Translate(displacements[j]);
//            std::cout << "Ghost After: " << ghost->ToString() << std::endl;
            ghosts.push_back(ghost);
        }
    }

//    for(uint i=0;i<ghosts.size();i++)
//        std::cout << "Ghosts Returned: " << ghosts[i]->ToString() << std::endl;

    return ghosts;
}

std::string MCDriver::ToString()
{
    std::string s = "";

    // First, the cell tensor
    s += this->cell.ToString();
    s += "\n";
    
    // Now the particles
    for(uint i=0;i<this->particles.size();i++)
    {
        s += string("particle: ") + this->particles[i]->ToString() + "\n";
    }

    // Now the ghosts
    std::vector<Tetrahedron*> ghosts = this->GetPeriodicGhosts();
    for(uint i=0;i<ghosts.size();i++)
    {
        s += string("ghost: ") + ghosts[i]->ToString() + "\n";
    }
    __FreeGhosts(ghosts);

    return s;
}

