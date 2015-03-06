#include "Tetrahedron.h"

// ========================================================================================================
// Constructor - build a new Tetrahedron instance from a point (the COM) + 3D rotation
// ========================================================================================================
Tetrahedron::Tetrahedron(Vector origin, float roll, float pitch, float yaw)
{
    //Start from a standard Tetrahedron centered at (0,0,0), then apply the origin translation and rotation
    vertices.push_back(Vector(0.5, 0, -0.5/sqrt(2.0)));
    vertices.push_back(Vector(-0.5, 0, -0.5/sqrt(2.0)));
    vertices.push_back(Vector(0, 0.5, 0.5/sqrt(2.0)));
    vertices.push_back(Vector(0, -0.5, 0.5/sqrt(2.0)));

    this->Translate(origin);
    this->Rotate(roll, pitch, yaw);
}

Vector Tetrahedron::GetCOM()
{
    Vector com(0,0,0);
    for(unsigned int i=0;i<this->vertices.size();i++) 
        com += this->vertices[i];
    com /= this->vertices.size();

    return com;
}

// ========================================================================================================
// Intersects - Returns `true` if the two Tetrahedra (`this` and `t2`) are intersecting, `false` otherwise
// ========================================================================================================
bool Tetrahedron::Intersects(Tetrahedron *t2)
{
    //TODO: Implement
    if ((this->GetCOM() - t2->GetCOM()).norm() < .5)
        return true;

    return false;
}

// ========================================================================================================
// Rotate - Rotate the particle in place (intrinsic) by translating to origin, applying rotation matrices
//          then translating back
// ========================================================================================================
void Tetrahedron::Rotate(Matrix rot_matrix)
{
    //TODO: Change storage of Tetrahedra to 1 center of mass vector + 4 delta vectors from the COM to each vertex.
    // this will simplify rotating about an intrinsic axis, etc.

    Vector com = -1.0 * this->GetCOM();
    this->Translate(com);

    // Apply rotation
    for(unsigned int i=0;i<this->vertices.size();i++)
        this->vertices[i] = rot_matrix * this->vertices[i];

    // Translate back
    com *= -1;
    this->Translate(com);
}

Matrix Tetrahedron::Rotate(float roll, float pitch, float yaw)
{
    // Compute rotation matrix
    // *Note*: this bit was pulled verbatim from (http://stackoverflow.com/questions/21412169/creating-a-rotation-matrix-with-pitch-yaw-roll-using-eigen)
    Eigen::AngleAxisd rollAngle(roll, Eigen::Vector3d::UnitZ());
    Eigen::AngleAxisd yawAngle(yaw, Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd pitchAngle(pitch, Eigen::Vector3d::UnitX());

    Eigen::Quaternion<double> q = rollAngle * yawAngle * pitchAngle;

    this->Rotate(q.matrix());

    return q.matrix();
}

// ========================================================================================================
// ToString - Return a formatted string of this particle's coordinates (v1x, v1y, v1z, v2x, v2y, ... )
// ========================================================================================================
std::string Tetrahedron::ToString()
{
    std::string s = "";
    for(unsigned int i=0;i<this->vertices.size();i++)
    {
        for(int j=0;j<3;j++)
            s += std::to_string(this->vertices[i][j]) + " ";
    }

    return s;
}

// ========================================================================================================
// Translate - Translate the particle 
// ========================================================================================================
void Tetrahedron::Translate(Vector &v)
{
    for(unsigned int i=0;i<this->vertices.size();i++)
        this->vertices[i] += v;
}
// Quick handler for pointers
void Tetrahedron::Translate(Vector *v){this->Translate(*v);}
