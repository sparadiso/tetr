#include "Globals.h"
#include "MCDriver.h"
#include <iostream>
#include <stdio.h>

MCDriver::MCDriver(int n_particles, 
                Real dr, Real droll, Real dpitch, Real dyaw, // Particle Move Parameters
                Real dx, Real dy, Real dz) : cell() // Cell Move Parameters
{
    moves.push_back(new CellShapeMove(&this->cell, dx, dy, dz));

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
        moves.push_back(new ParticleTranslation(t, dr));

        moves.back()->Apply();
        while( this->CheckCollisionsWith(t, this->GetPeriodicGhosts()) )
        {
            moves.back()->Apply();
        }
        moves.push_back(new ParticleRotation(t, droll, dpitch, dyaw));
    }
}

void __FreeGhosts(std::vector<Tetrahedron*> ghosts)
{
    for (uint j=0;j<ghosts.size();j++) 
        delete ghosts[j];
}
bool MCDriver::CheckCollisionsWith(Tetrahedron *t, std::vector<Tetrahedron*> ghosts)
{
    // First, check collisions with other particles
    for(uint i=0;i<particles.size();i++)
        if (t->Intersects(particles[i]) && (t->GetCOM() - particles[i]->GetCOM()).norm() > 1E-5 )
        {
            __FreeGhosts(ghosts);
            return true;
        }

    // Now check collisions with periodic images
    for(uint i=0;i<ghosts.size();i++)
        if (t->Intersects(ghosts[i]))
        {
            __FreeGhosts(ghosts);
            return true;
        }

    // If no collisions, free up the ghost list then return
    __FreeGhosts(ghosts);

    return false;
}

void MCDriver::MakeMove()
{
    // First, choose the move to make

    int move_index = rand() % this->moves.size();

    Move *m = this->moves[move_index];
    m->Apply();

    bool accepted = true;

    // If this is a ParticleMove, then check collisions with the moved particle
    try 
    { 
        ParticleMove *pmove = reinterpret_cast<ParticleMove*>(m); 
        accepted = this->CheckCollisionsWith(pmove->particle, this->GetPeriodicGhosts());
    } 
    catch(int e){}

    //CHECK IF VALID
    if(!accepted)
        m->Undo();
}

// NOT FINISHED. 
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
        s += this->particles[i]->ToString();
    }

    return s;
}

