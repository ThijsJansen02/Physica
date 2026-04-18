
import os
import sys
import subprocess
import RpGui as rp

# caution: path[0] is reserved for script path (or '' in REPL)
sys.path.insert(1, 'res')

import execlib

execlib.pri()

def openandprint(path):
    with open(path, "r") as file:
        for line in file:
            print(line.strip())

def ls():
    os.system("dir")


def printversion():
    print(f"version: {sys.version}")

rp.setFilterCutoff(0, 500)
rp.setFilterCutoff(1, 1000)
rp.setFilterCutoff(2, 100)

rp.setFilterQfactor(0, 100)
rp.setFilterQfactor(1, 3.75)
rp.setFilterQfactor(2, 100)