from vis import *
import glob

def ParseCell(fname):
    # Return cell tensor and particle positions
    with open(fname) as f:
        e0 = np.array(f.readline().split(), float)
        e1 = np.array(f.readline().split(), float)
        e2 = np.array(f.readline().split(), float)
        
        particles = []
        for line in f:
            line = line.split()
            if line[0]=='ghost:':
                break
            particles.append(np.array(line[1:], float))

    return [e0, e1, e2], particles

# Quick trick for enumerating all points on the cube
def reset():
    for i in range(8):
        x, y, z = [int(j) for j in "{:03d}".format(int(bin(i)[2:]))]
        scale=1
        mlab.points3d(scale*x, scale*y, scale*z, scale_factor=.1)

i=0

def MakeMultipleImages():
    tr = [a+b+c for a, b, c in zip([e0,0,0,e0,e0,0,e0], [0,e1,0,e1,0,e1,e1], [0,0,e2,0,e2,e2,e2])]
    [cell.paint(e) for e in tr]

def ViewFile(fname, show=True, PaintGhosts=True):
    with open(fname) as f:
        # First, read off the cell tensor
        e0 = np.array(f.readline().split(), float)
        e1 = np.array(f.readline().split(), float)
        e2 = np.array(f.readline().split(), float)

        #reset()

        cell = Cell(e0, e1, e2)
        cell.paint()

#        MakeMultipleImages()

        for line in f.readlines():
            l = line.split()
            t = Tetrahedron(*np.array(l[1:]))

            tag = l[0].strip()
            if tag == "ghost:":
                if PaintGhosts:
                    t.paint(0.1, colormap='spectral')
            else:
#                [t.paint(0.25, origin=e, colormap='autumn') for e in tr]
                t.paint(1.0, colormap='gray')

        if show:
            mlab.show()

def CreateImagesFromFiles(fnames, **kwargs):
    i=0
    for fname in sorted(glob.glob("../{}*".format(fnames))):
        ViewFile(fname, False, **kwargs)

        mlab.savefig("out_{:02d}.png".format(i))
        i+=1

        mlab.clf()
