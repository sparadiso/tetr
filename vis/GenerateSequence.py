#!/usr/bin/python
import test
import sys

test.CreateImagesFromFiles("output/{}".format(sys.argv[1]), PaintGhosts=False)

