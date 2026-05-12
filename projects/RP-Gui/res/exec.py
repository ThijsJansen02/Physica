
import os
import sys
import subprocess
import RpGui as rp
import numpy as np
import hostimgui as imgui

# caution: path[0] is reserved for script path (or '' in REPL)
sys.path.insert(1, 'res')

import sys, os
sys.stdout = open('CONOUT$', 'w')
sys.stderr = open('CONOUT$', 'w')


#tf = rp.getTransferFunction("example function")
#tf.sentCommandToRp("example command")

freq, mag, phase = np.loadtxt("res/traces.csv", delimiter=",", skiprows=9, unpack=True)

mag = mag

#rp.removePlot("example plot")
rp.addPlot(np.log10(freq), mag, np.deg2rad(phase), "example plot", rp.Vec4(1.0, 0.0, 0.0, 1.0))
