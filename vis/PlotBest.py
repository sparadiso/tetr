#!/usr/bin/python
import sys
import glob
from vis import *
from mayavi import mlab

best_fname = sorted(glob.glob("../output/best*"))[-1]

cell, particles = LoadFile(best_fname)

# Paint the scene
cell.paint()

for p in particles:
    p.paint(alpha=1.0)

mlab.show()

quit()

for i in range(-1,2,1):
    for j in range(-1,2,1):
        for k in range(-1,2,1):
            for p in particles:
                p.translate(np.dot(cell.h, [i,j,k]))
                p.paint(0.5)
                p.translate(-np.dot(cell.h, [i,j,k]))

