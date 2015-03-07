#include "Shape.h"

Vector Shape::GetCOM()
{
    Vector com(0,0,0);
    for(unsigned int i=0;i<this->vertices.size();i++)
        com += this->vertices[i];
    com /= this->vertices.size();

    return com;
}

Shape::~Shape()
{
}

void Shape::Rotate(Matrix rot_matrix)
{
    Vector com = -1.0 * this->GetCOM();
    this->Translate(com);

    // Apply rotation
    for(unsigned int i=0;i<this->vertices.size();i++)
        this->vertices[i] = rot_matrix * this->vertices[i];

    // Translate back
    com *= -1;
    this->Translate(com);
}

// ========================================================================================================
// Rotate - Rotate the particle in place (intrinsic) by translating to origin, applying rotation matrices
//          then translating back
// ========================================================================================================
Matrix Shape::Rotate(float roll, float pitch, float yaw)
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
std::string Shape::ToString()
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
void Shape::Translate(Vector v)
{
    for(unsigned int i=0;i<this->vertices.size();i++)
        this->vertices[i] += v;
}
