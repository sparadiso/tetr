#!/usr/bin/python
import test
import sys
import glob

best_fname = sorted(glob.glob("../output/best*"))[-1]

test.ViewFile(best_fname, PaintGhosts=False)
