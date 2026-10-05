import numpy as np
import os
import sys

def openandprint(path):
    with open(path, "r") as file:
        for line in file:
            print(line.strip())

def ls():
    os.system("dir")


def printversion():
    print(f"version: {sys.version}")


