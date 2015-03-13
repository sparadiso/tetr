#pragma once

#define PI 3.14159265359

#include <Eigen/Dense>
#include <iostream>
#include <stdio.h>

typedef Eigen::Matrix3d Matrix;
typedef Eigen::Vector3d Vector;

typedef float Real;

// Simple uniform RNG - real MC would require a better RNG, but this is fine for our purposes. 
Real u(Real lower, Real upper);

