from scipy.spatial import ConvexHull
from mayavi import mlab
import numpy as np

class Cell(object):
    def __init__(self, e0, e1, e2):
        self.origin = [0,0,0]
        self.h = [np.array(e) for e in [e0,e1,e2]]
        self.h_inv = np.linalg.inv(self.h)

    def getVolume(self):
        return np.dot(self.h[0], np.cross(self.h[1], self.h[2]))

    def CreateGhost(self, pt):
        s = np.dot(pt, self.h_inv)
        pt_new = np.dot(np.mod(s, 1), self.h)
        return pt_new

    def is_inside(self, pt):
        s = np.dot(pt, self.h_inv)

        mlab.points3d(*pt, scale_factor=0.1)

    def paint(self, origin=[0,0,0]):
        origin = np.array(origin)

        pts = []

        for i in range(8):
            indices = [int(j) for j in "{:03d}".format(int(bin(i)[2:]))]
    
            pt = [u*i for u, i in zip(self.h, indices)]
            pt = np.sum(pt, axis=0)
            mlab.points3d(*pt, scale_factor=0.05)
            pts.append(pt)

        self.hull = ConvexHull(pts)

        x = self.hull.points[:, 0] + origin[0]
        y = self.hull.points[:, 1] + origin[1]
        z = self.hull.points[:, 2] + origin[2]

        o = origin
        [mlab.plot3d(*np.transpose([o, e+o])) for e in self.h]
        mlab.triangular_mesh(x,y,z,self.hull.simplices,colormap='gray', opacity=0.25)

class Tetrahedron(object):
    # pts is packed like the requested output: x1, y1, z1, x2, y2, z2, ... z4
    def __init__(self, *pts):
        # default tetrahedron
        self.points = 1/2.0 * np.array([[1,0,-1./np.sqrt(2)], 
                       [-1, 0, -1/np.sqrt(2.)], 
                       [0, 1, 1./np.sqrt(2.)], 
                       [0, -1, 1./np.sqrt(2.)]])

        if len(pts) > 0:
            self.points = []
            for i in range(4):
                self.points.append( [ pts[3*i+j] for j in range(3) ] )

        self.points = np.array(self.points, float)

    def translate(self, *dr):
        for p in self.points:
            p += np.array(dr)

    def paint(self, alpha=0.75, origin=[0,0,0], colormap='gray'):
        origin = np.array(origin)

        self.hull = ConvexHull(self.points)

        x = self.hull.points[:, 0] + origin[0]
        y = self.hull.points[:, 1] + origin[1]
        z = self.hull.points[:, 2] + origin[2]

        if len(self.points) == 1:
            mlab.points3d(*np.transpose(self.points))
        else:
            mlab.triangular_mesh(x,y,z,self.hull.simplices,colormap=colormap, opacity=alpha)
