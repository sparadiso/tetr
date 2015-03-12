#pragma once
#include "Shape.h"

class Triangle
{
    public:
    
    Real *vertex, *edge1, *edge2;

    Triangle(Vector e0, Vector e1, Vector e2)
    {   
        this->vertex = new Real[3];
        this->edge1 = new Real[3];
        this->edge2 = new Real[3];

        this->Update(e0, e1, e2);
    }
    ~Triangle()
    {
        delete [] vertex;
        delete [] edge1;
        delete [] edge2;
    }
    
    void Update(Vector &e0, Vector &e1, Vector &e2)
    {
        // Set the vertex to the first corner
        for(int i=0;i<3;i++)
            vertex[i] = e0[i];

        // Get the edges by vector subtraction
        Vector dr = e1 - e0;
        for(int i=0;i<3;i++)
            edge1[i] = dr[i];

        // Get the edges by vector subtraction
        dr = e2 - e0;
        for(int i=0;i<3;i++)
            edge2[i] = dr[i];
    }
    std::string ToString()
    {
        std::string str = "";
        str += "Vertex:";
        for(int i=0;i<3;i++)
            str += std::to_string(vertex[i]) + ", ";
        str += "\n";

        str += "Edge1:";
        for(int i=0;i<3;i++)
            str += std::to_string(edge1[i]) + ", ";
        str += "\n";

        str += "Edge2:";
        for(int i=0;i<3;i++)
            str += std::to_string(edge2[i]) + ", ";
        str += "\n";
        return str;
    }
};

class Tetrahedron: public Shape
{
    public:
    std::vector<Triangle*> triangles;
    
    Tetrahedron(Vector origin, float roll=0, float pitch=0, float yaw=0);
    Tetrahedron(Tetrahedron &t);
    ~Tetrahedron();

    bool Intersects(Shape *t2);
    Real GetVolume();

    void UpdateTriangles();
    void Rotate(Real roll, Real pitch, Real yaw);
    void Translate(Vector v);
};
