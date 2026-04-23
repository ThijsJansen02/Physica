
import os
import sys
import subprocess
import RpGui as rp
import numpy as np

# caution: path[0] is reserved for script path (or '' in REPL)
sys.path.insert(1, 'res')

import sys, os
sys.stdout = open('CONOUT$', 'w')
sys.stderr = open('CONOUT$', 'w')


def openandprint(path):
    with open(path, "r") as file:
        for line in file:
            print(line.strip())

def ls():
    os.system("dir")


def printversion():
    print(f"version: {sys.version}")

x = np.linspace(0, 10, 100, dtype=np.float32)
y = 2 * np.sin(x)

rp.addPlot(x, y, "sin(x)")

#rp.setFont("c:/windows/fonts/Times.ttf", 512, 32)