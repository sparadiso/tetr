#include "Tetrahedron.h"
#include "Collision.h"

// ========================================================================================================
// Constructor - build a new Tetrahedron instance from a point (the COM) + 3D rotation
// ========================================================================================================
Tetrahedron::Tetrahedron(Vector origin, float roll, float pitch, float yaw): Shape()
{
    //Start from a standard Tetrahedron centered at (0,0,0), then apply the origin translation and rotation
    vertices.push_back(Vector(0.5, 0, -0.5/sqrt(2.0)));
    vertices.push_back(Vector(-0.5, 0, -0.5/sqrt(2.0)));
    vertices.push_back(Vector(0, 0.5, 0.5/sqrt(2.0)));
    vertices.push_back(Vector(0, -0.5, 0.5/sqrt(2.0)));

    // Initialize the 4-triangle representation of this tetrahedron for collision detection
    this->triangles.push_back(new Triangle(this->vertices[0], this->vertices[1], this->vertices[2]));
    this->triangles.push_back(new Triangle(this->vertices[0], this->vertices[1], this->vertices[3]));
    this->triangles.push_back(new Triangle(this->vertices[0], this->vertices[2], this->vertices[3]));
    this->triangles.push_back(new Triangle(this->vertices[1], this->vertices[2], this->vertices[3]));

    this->Translate(origin);
    this->Rotate(roll, pitch, yaw);
}

// Copy constructor
Tetrahedron::Tetrahedron(Tetrahedron &t)
{
    // Copy the vertices
    for(uint i=0;i<t.vertices.size();i++)
        vertices.push_back(t.vertices[i]);

    // Initialize the 4-triangle representation of this tetrahedron for collision detection
    this->triangles.push_back(new Triangle(this->vertices[0], this->vertices[1], this->vertices[2]));
    this->triangles.push_back(new Triangle(this->vertices[0], this->vertices[1], this->vertices[3]));
    this->triangles.push_back(new Triangle(this->vertices[0], this->vertices[2], this->vertices[3]));
    this->triangles.push_back(new Triangle(this->vertices[1], this->vertices[2], this->vertices[3]));
}
Tetrahedron::~Tetrahedron()
{
    for(uint i=0;i<this->triangles.size();i++)
        delete this->triangles[i];
}

// ========================================================================================================
// Intersects - Returns `true` if the two Tetrahedra (`this` and `t2`) are intersecting, `false` otherwise
// ========================================================================================================
bool Tetrahedron::Intersects(Shape *shape)
{
    // This only works for tetrahedra, so just reinterpret_cast here
    Tetrahedron *t2 = reinterpret_cast<Tetrahedron*>(shape);

    // If the centers of mass of the two tetrahedra are further than the diameter of the sphere that circumscribes a regular tetrahedron, then no collision is possible
    if ((this->GetCOM() - t2->GetCOM()).norm() > sqrt(6.)/2.0)
        return false;

    // Check each pair of triangles making up the two tetrahedra and check if they're intersecting
    for(int i=0;i<4;i++)
    for(int j=0;j<4;j++)
    {
        Triangle *tri_1 = this->triangles[i];
        Triangle *tri_2 = t2->triangles[j];

        // If these two triangles intersect, then return true for collision
        if(tr_tri_intersect3D(tri_1->vertex, tri_1->edge1, tri_1->edge2,
                              tri_2->vertex, tri_2->edge1, tri_2->edge2) != 0)
        {
            return true;
        }
    }
        
    return false;
}

// Quick helper
Real Tetrahedron::GetVolume()
{
    return 1.0 / (6*sqrt(2.0));
}

void Tetrahedron::UpdateTriangles()
{
    this->triangles[0]->Update(this->vertices[0], this->vertices[1], this->vertices[2]);
    this->triangles[1]->Update(this->vertices[0], this->vertices[1], this->vertices[3]);
    this->triangles[2]->Update(this->vertices[0], this->vertices[2], this->vertices[3]);
    this->triangles[3]->Update(this->vertices[1], this->vertices[2], this->vertices[3]);
}

// Intercept all Rotate/Translate calls and update the triangle representations so we're ready for collision detection after each move
void Tetrahedron::Rotate(Real roll, Real pitch, Real yaw)
{
    Shape::Rotate(roll, pitch, yaw);
    this->UpdateTriangles();
}
void Tetrahedron::Translate(Vector dr)
{
    Shape::Translate(dr);
    this->UpdateTriangles();
}
