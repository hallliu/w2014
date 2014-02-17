#!/usr/bin/python2
import scipy.optimize as so
import numpy as np
from ctypes import *

ctr = CDLL('./counter.so')
ctr.parallel_work_test.restype = c_double
ctr.parallel_time_test.restype = c_int

class BackoffConfig(Structure):
    _fields_ = [('min_delay', c_int), ('max_delay', c_int)]

def get_backoff_times(times):
    info = BackoffConfig(int(times[0]*1000), int(times[1]*1000))
    incrs = 0;
    for i in range(20):
        incrs += ctr.parallel_time_test(10, 4, 'backoff', byref(info))
    return incrs / 20

def get_backoff_work(times):
    info = BackoffConfig(int(times[0]*1000), int(times[1]*1000))
    times = 0;
    for i in range(20):
        times += ctr.parallel_work_test(1000000, 4, 'backoff', byref(info))
    return times / 20

for i in range(6):
    print so.fmin_powell(get_backoff_work, np.array([0.02, 2]))
    print so.fmin_powell(get_backoff_times, np.array([0.02, 2]))
