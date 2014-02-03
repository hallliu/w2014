#!/usr/bin/python2
import numpy as np
import matplotlib.pyplot as plt
from subprocess import check_output
from itertools import product
import os

def parallel_overhead(exponent):
    n_iters = 5
    serial_times = np.zeros((3, 6), dtype='float64')
    serial_queue_times = np.zeros((3, 6), dtype='float64')
    means = [25, 50, 100, 200, 400, 800]
    srcs = [1, 8, 16]

    for n_ind, m_ind in product(range(3), range(6)):
        n_pkts = 2**exponent / (means[m_ind] * srcs[n_ind])
        for i in range(n_iters):
            serial_times[n_ind, m_ind] += float(check_output(['./firewall', '0', str(n_pkts), str(srcs[n_ind]), str(means[m_ind]), '1', str(i), '32']))
            serial_queue_times[n_ind, m_ind] += float(check_output(['./firewall', '1', str(n_pkts), str(srcs[n_ind]), str(means[m_ind]), '1', str(i), '32']))

    serial_times /= n_iters
    serial_queue_times /= n_iters 

    os.mkdir('results/parallel_overhead')
    np.savetxt('results/parallel_overhead/serial_times.csv', serial_times, delimiter=',')
    np.savetxt('results/parallel_overhead/serial_queue_times.csv', serial_queue_times, delimiter=',')

def dispatcher_rate(exponent):
    n_iters = 5
    srcs = [1, 2, 4, 8, 16, 32]
    runtimes = np.zeros(6, dtype='float64')

    for ind, n in enumerate(srcs):
        n_pkts = 2 ** exponent / n
        for i in range(n_iters):
            runtimes[ind] += float(check_output(['./firewall', '2', str(n_pkts), str(n), '1', '1', str(i), '32']))

    runtimes /= n_iters
    np.savetxt('results/dispatcher_rate.csv', runtimes, delimiter=',')

def unif_speedup(exponent):
    workloads = [1000, 2000, 4000, 8000]
    srcs = [1, 2, 4, 8, 16, 32, 64]
    n_iters = 5


if __name__ == '__main__':
    os.mkdir('results')
    parallel_overhead(24)
    dispatcher_rate(20)
