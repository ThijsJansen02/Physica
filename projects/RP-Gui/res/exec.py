
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

input, output = np.loadtxt("testdata.csv", delimiter=",", skiprows=1, unpack=True)

input_fft = np.fft.fft(input)
output_fft = np.fft.fft(output)

in_out = output_fft / input_fft

freq = np.fft.fftfreq(len(input), d=64/125000000)

#rp.removePlot("example plot")
rp.addPlot(np.log10(freq[::100]), 20 * np.log10(np.abs(in_out[::100])), np.angle(in_out[::100]), "example plot", rp.Vec4(1.0, 0.0, 0.0, 1.0))
