from enum import IntEnum
import numpy as np


class FilterType(IntEnum):
    LOWPASS : int
    HIGHPASS : int
    BANDSTOP : int
    BANDPASS : int
    ALLPASS : int
    RESONANCE_ANTI_RESONANCE : int
    COEFFICIENTS : int

class Vec4:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0, w: float = 0.0) -> None: ...
    x: float
    y: float
    z: float
    w: float

class Filter:
    cutoff : float
    Qfactor : float
    df : float
    antiQfactor : float
    type : FilterType    

    def getCoeffs(self) -> np.ndarray: ...
    def recalculate(self) -> None: ...


class TransferFunction:
    def writeToRp(self, filter: Filter) -> None: ...
    def getFilters(self) -> list[Filter]: ...
    def sentCommandToRp(self, command: str) -> None: ...

def getTransferFunction(name : str) -> TransferFunction: ...
def addPlot(freq: np.ndarray, magnitude: np.ndarray, phase: np.ndarray, name: str, color: Vec4) -> None: ...
def setTitle(title: str) -> None: ...


def removePlot(name: str) -> None: ...