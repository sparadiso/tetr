# tetr

An isothermal-isobaric ensemble Metropolis Monte Carlo code for the Citrine Technical Challenge (#2). Written in C++ by an obviously Java trained programmer.

## Quick layout

Two main class clusters: **Moves** and **Shapes**

The driver class **MCDriver** populates a list of **Shape**s, initializes a **Cell**, and performs MC **Moves**.

All collision detection is handled in the derived shape classes (**Sphere** and **Tetrahedron**) with help from "Collisions.cpp" pulled from the challenge site.

Results of the simulation are printed to files with the following name convention: `output/best%04d`. The configuration for the optimal packing found will be in the last file in this list.

## Summary of results

Best recorded **Sphere** packing (6 spheres): 73.33% 

Best recorded **Tetrahedron** packing (4 tetrahedra): 82.5% - a bit below the best known packing of ~85% (and not the same structure).

### Parameter Explanation

There are a number of cmd line arguments for setting runtime variables:

#### Main System Variables: n_particles, n_steps, n_drivers, p{i}

#### Main Move Parameters: p_cell_move, ProjectionThreshold, dcell, dr

n_particles - Number of particles in the cell

n_steps - Number of MC moves to perform

n_drivers - Number of parallel MCDrivers to run (each system runs simultaneously, with the occasional parallel tempering move attempted to swap the pressures of two randomly selected systems)

p{i} - For each driver (of which there are `n_drivers`), a corresponding p{i} (p0, p1, p2, etc) must be supplied which sets the initial pressure of each system.

p_cell_move - Probability of choosing a cell move v.s. a particle move

ProjectionThreshold - A stability parameter limiting the internal angles of the fundamental cell from becoming too acute. If your MC trajectories diverge, use a smaller value. If you don't find dense solutions, try a larger value. I've found .7 to be a good compromise.

dcell - Maximum size of a cell shape move. (Larger values lead to less efficient MC sampling and high move rejection, smaller values lead to poor phase space sampling).

dr - Maximum particle displacement/rotation move size (measured in radians for rotations, edge lengths for displacements).

## Usage

First, you'll have to compile it. Assuming you have the standard libraries installed with gcc 4.7 or higher, the project should compile by just typing `make` in the root.

Example Usage (with suggested values):

./main n_particles 4 n_steps 3000000 n_drivers 4 p0 50 p1 250 p2 500 p3 1000 dcell .01 dr .02 ProjectionThreshold 0.7

*note* the code can be run with **Tetrahedra** or **Spheres**, but this choice must be made by changing `ChosenShape` in src/main.cpp and recompiling. 
