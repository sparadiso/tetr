from vis import *

# Quick trick for enumerating all points on the cube
def reset():
    for i in range(8):
        x, y, z = [int(j) for j in "{:03d}".format(int(bin(i)[2:]))]
        scale=1
        mlab.points3d(scale*x, scale*y, scale*z, scale_factor=.1)

with open("../pts") as f:
    # First, read off the cell tensor
    e0 = np.array(f.readline().split(), float)
    e1 = np.array(f.readline().split(), float)
    e2 = np.array(f.readline().split(), float)
    cell = Cell(e0, e1, e2)
    cell.paint()

    i=0
    for line in f.readlines():
        l = line.split()
        t = Tetrahedron(*np.array(l[1:]))
#        reset()
        tag = l[0].strip()
        if tag == "ghost:":
            t.paint(0.25)
        else:
            t.paint(0.85)

#        mlab.savefig("out_{:02d}.png".format(i)); i+=1
#        mlab.clf()
mlab.show()
