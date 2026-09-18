
import os
import sys
import subprocess
import RpGui as rp
import numpy as np
import hostimgui as imgui
from pathlib import Path

# caution: path[0] is reserved for script path (or '' in REPL)
sys.path.insert(1, 'res')

import sys, os
sys.stdout = open('CONOUT$', 'w')
sys.stderr = open('CONOUT$', 'w')

def OpenCSVandWriteToGUI(path):
    print(path)
    data_rp = np.loadtxt(path, delimiter=",", skiprows=20, unpack=True)

    #rp.removePlot("measured frequency response")
    rp.addPlot(np.log10(data_rp[0]), data_rp[1], np.deg2rad(data_rp[2]), Path(path).stem, rp.Vec4(1.0, 1.0, 0.0, 1.0))
    rp.setTitle("CIC compiler compare test")


