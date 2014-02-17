import scipy.optimize as so
from ctypes import *

class BackoffConfig(Structure):
    _fields_ = [('min_delay', c_int), ('max_delay', c_int)]


