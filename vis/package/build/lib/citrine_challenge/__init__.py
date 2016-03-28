from scipy.spatial import ConvexHull
from mayavi import mlab
import numpy as np


class Cell(object):
    """

    The unit cell, with support for computing the volume and painting the box using mayavi.

    """

    def __init__(self, e0, e1, e2):
        self.origin = [0, 0, 0]
        self.h = [np.array(e) for e in [e0, e1, e2]]

    def get_volume(self):
        """
        Get the volume of the cell.

        :return: the cell volume
        """
        return np.dot(self.h[0], np.cross(self.h[1], self.h[2]))

    def paint(self, origin=[0, 0, 0]):
        """
        Paints the 3 unit cell tensors to the current mayavi canvas (creates one if none exists) and shades
        the walls.

        :param origin: the origin of the cell. Useful if you want to paint the periodic ghost cells.
        """

        origin = np.array(origin)

        pts = []

        for i in range(8):
            indices = [int(j) for j in "{:03d}".format(int(bin(i)[2:]))]

            pt = [u * i for u, i in zip(self.h, indices)]
            pt = np.sum(pt, axis=0)
            mlab.points3d(*pt, scale_factor=0.05)
            pts.append(pt)

        self.hull = ConvexHull(pts)

        x = self.hull.points[:, 0] + origin[0]
        y = self.hull.points[:, 1] + origin[1]
        z = self.hull.points[:, 2] + origin[2]

        o = origin
        [mlab.plot3d(*np.transpose([o, e + o])) for e in self.h]
        mlab.triangular_mesh(x, y, z, self.hull.simplices, colormap='gray', opacity=0.25)


class Tetrahedron(object):
    """
    A (paintable) tetrahedron particle.
    """

    def __init__(self, *pts):
        """
        Instantiate a tetrahedron from a list of coordinates.

        :param pts: pts array (x1, y1, z1, x2, y2, z2, ... z4)
        """
        self.points = []
        for i in range(len(pts) / 3):
            self.points.append([pts[3 * i + j] for j in range(3)])

        self.points = np.array(self.points, float)

    def translate(self, dr):
        """
        Translate the particle's center of mass by `dr`

        :param dr:
        :return:
        """

        assert len(dr) == 3, "translation parameter (dr) must be a 3-element array [dx, dy, dz]"

        for p in self.points:
            p += np.array(dr)

    def paint(self, alpha=0.75, origin=[0, 0, 0], colormap='gray'):
        """
        Paint the tetrahedron.

        :param alpha: transparency
        :param origin: particle origin (assumed [0,0,0])
        :param colormap: (String) mlab colormap
        """
        origin = np.array(origin)

        if len(self.points) == 1:
            mlab.points3d(*np.transpose(self.points), colormap=colormap, opacity=alpha, resolution=16)
        else:
            self.hull = ConvexHull(self.points)

            x = self.hull.points[:, 0] + origin[0]
            y = self.hull.points[:, 1] + origin[1]
            z = self.hull.points[:, 2] + origin[2]

            mlab.triangular_mesh(x, y, z, self.hull.simplices, colormap=colormap, opacity=alpha)


def load_solution(fname):
    """
    Load a candidate's solution and parse the unit cell and particle locations out.

    :param fname: full path to the solution file being visualized
    :return: the unit cell and particles list
    """
    with open(fname) as f:
        # First, read off the cell tensor
        e0 = np.array(f.readline().split(), float)
        e1 = np.array(f.readline().split(), float)
        e2 = np.array(f.readline().split(), float)

        cell = Cell(e0, e1, e2)

        particles = []

        # Read off the tetrahedra
        for line in f.readlines():
            l = line.split()
            particles.append(Tetrahedron(*np.array(l)))

    return cell, particles

def display_solution(fname):
    """
    Quickie to plot
    :param fname:
    :return:
    """
