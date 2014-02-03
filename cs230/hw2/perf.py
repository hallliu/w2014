#!/usr/bin/python2
import numpy as np
import matplotlib.pyplot as plt
from subprocess import check_output
from itertools import product

def parallel_overhead():
    n_iters = 5
    serial_times = np.empty((3, 6), dtype='float64')
    serial_queue_times = np.empty((3, 6), dtype='float64')
    means = [25, 50, 100, 200, 400, 800]
    srcs = [1, 8, 16]

    for m_ind, n_ind in product(range(3), range(6)):
        n_pkts = 2**20 / (means[m_ind] * srcs[n_ind])
        for i in range(n_iters):
            serial_times[n_ind, m_ind] += float(check_output(['./firewall', '0', str(n_pkts), str(srcs[n_ind]), str(means[m_ind]), '1', str(i), '32']))
            serial_queue_times[n_ind, m_ind] += float(check_output(['./firewall', '1', str(n_pkts), str(srcs[n_ind]), str(means[m_ind]), '1', str(i), '32']))

    serial_times /= n_iters
    serial_queue_times /= n_iters 
    import ipdb;ipdb.set_trace()



