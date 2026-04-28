
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


def openandprint(path):
    with open(path, "r") as file:
        for line in file:
            print(line.strip())

def ls():
    os.system("dir")


def printversion():
    print(f"version: {sys.version}")


style =  imgui.getStyle()

light = imgui.Vec4(0.2, 0.2, 0.2, 1.0)
normal = imgui.Vec4(0.1, 0.1, 0.1, 1.0)
dark = imgui.Vec4(0.01, 0.01, 0.01, 1.0)

white = imgui.Vec4(1.0, 1.0, 1.0, 1.0)

imgui.setColor(imgui.Col.Text, white)

imgui.setColor(imgui.Col.WindowBg, dark)
imgui.setColor(imgui.Col.FrameBg, normal)
imgui.setColor(imgui.Col.FrameBgHovered, light)
imgui.setColor(imgui.Col.FrameBgActive, imgui.Vec4(0.3, 0.3, 0.3, 1.0))

imgui.setColor(imgui.Col.TitleBg, imgui.Vec4(0.01, 0.01, 0.01, 1.0))
imgui.setColor(imgui.Col.TitleBgActive, imgui.Vec4(0.01, 0.01, 0.01, 1.0))
imgui.setColor(imgui.Col.Button, light)
imgui.setColor(imgui.Col.ButtonHovered, imgui.Vec4(0.2, 0.2, 0.2, 1.0))
imgui.setColor(imgui.Col.ButtonActive, imgui.Vec4(0.3, 0.3, 0.3, 1.0))
imgui.setColor(imgui.Col.Border, imgui.Vec4(0.1, 0.1, 0.1, 1.0))
imgui.setColor(imgui.Col.PopupBg, imgui.Vec4(0.01, 0.01, 0.01, 1.0))

imgui.setColor(imgui.Col.Header, imgui.Vec4(0.1, 0.1, 0.1, 1.0))
imgui.setColor(imgui.Col.HeaderHovered, imgui.Vec4(0.2, 0.2, 0.2, 1.0))
imgui.setColor(imgui.Col.HeaderActive, imgui.Vec4(0.3, 0.3, 0.3, 1.0))

style.frame_rounding = 3.0
style.window_rounding = 3.0
style.scrollbar_rounding = 3.0
style.frame_border_size = 1.0
style.window_border_size = 0.0


imgui.getStyle
