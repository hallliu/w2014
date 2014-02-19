#!/usr/bin/python2
import numpy as np
import matplotlib.pyplot as plt
from subprocess import Popen, PIPE
from itertools import product
import os
import threading
import datetime

logfile = None
locks = ['TAS', 'backoff', 'mutex', 'Alock', 'CLH', 'MCS']
'''
Class that implements timeout procedures for our tests
'''
class RunSP(threading.Thread):
    def __init__(self, cmd, timeout):
        threading.Thread.__init__(self)
        self.cmd = cmd
        self.timeout = timeout

    def run(self):
        self.proc = Popen(self.cmd, stdout=PIPE)
        self.proc.wait()

    def Run(self):
        self.start()
        self.join(self.timeout)

        if self.is_alive():
            self.proc.kill()
            self.join()
            return 'TIMEOUT'
        else:
            return self.proc.stdout.read()

class TimeoutException(Exception):
    pass

def timeout_output(cmd, timeout):
    out = RunSP(cmd, timeout).Run()
    if out == 'TIMEOUT':
        return '-1000000'
    else:
        return out

def _log(msg):
    logfile.write('{0}      {1}'.format(datetime.datetime.utcnow(), msg))

def counter_overhead_time():
    locks_here = locks + ['noop']

    lock_tps = np.zeros(len(locks_here), dtype='float64')
    for (i, l) in enumerate(locks_here):
        for j in range(4):
            print('counter_overhead_time: lock {0} iteration {1}'.format(l, j))
            lock_tps[i] += float(timeout_output(['./perf_main', 'parallel_time', l, '2000', '1'], 3000))
            lock_tps[i] /= 2000

    lock_tps /= 4
    np.savetxt('results/counter_overhead_work.csv', lock_tps, delimiter=',')
    return lock_tps

def counter_overhead_work():
    count_to = 30000000
    locks_here = locks + ['noop']
    lock_tps = np.zeros(len(locks_here), dtype='float64')

    for (i, l) in enumerate(locks_here):
        for j in range(4):
            print('counter_overhead_work: lock {0} iteration {1}'.format(l, j))
            lock_tps[i] = count_to / float(timeout_output(['./perf_main', 'parallel_work', l, str(count_to), '1'], 3000))

    lock_tps /= 4
    np.savetxt('results/counter_overhead_time.csv', lock_tps, delimiter=',')
    return lock_tps


def parallel_overhead(exponent):
    n_iters = 5
    means = [25, 50, 100, 200, 400, 800]
    srcs = [1, 3, 7, 15]
    serial_times = np.zeros((len(srcs), len(means)), dtype='float64')
    serial_queue_times = np.zeros((len(srcs), len(means)), dtype='float64')

    for n_ind, m_ind in product(range(len(srcs)), range(len(means))):
        n_pkts = 2**exponent / (means[m_ind] * srcs[n_ind])
        _log('parallel overhead: n={0}, mean={1}\n'.format(srcs[n_ind], means[m_ind]))
        for i in range(n_iters):
            serial_times[n_ind, m_ind] += float(timeout_output(['./firewall', '0', str(n_pkts), str(srcs[n_ind]), str(means[m_ind]), '1', str(i), '32'], 2))
            serial_queue_times[n_ind, m_ind] += float(timeout_output(['./firewall', '1', str(n_pkts), str(srcs[n_ind]), str(means[m_ind]), '1', str(i), '32'], 2))

    serial_times /= n_iters
    serial_queue_times /= n_iters 

    np.savetxt('results/parallel_overhead/serial_times.csv', serial_times, delimiter=',')
    np.savetxt('results/parallel_overhead/serial_queue_times.csv', serial_queue_times, delimiter=',')

def dispatcher_rate(exponent):
    n_iters = 5
    srcs = [1, 2, 3, 4, 7, 9, 13, 15, 31]
    runtimes = np.zeros(len(srcs), dtype='float64')

    for ind, n in enumerate(srcs):
        n_pkts = 2 ** exponent / n
        _log('dispatcher rate: n={0}\n'.format(srcs[ind]))
        for i in range(n_iters):
            runtimes[ind] += float(timeout_output(['./firewall', '2', str(n_pkts), str(n), '1', '1', str(i), '32'], 2))

    runtimes /= n_iters
    np.savetxt('results/dispatcher_rate.csv', runtimes, delimiter=',')

def unif_speedup(exponent, upto):
    workloads = [1000, 2000, 4000, 8000]
    srcs = [1, 3, 7, 15, 31, 63][:upto]
    n_iters = 5

    serial_times = np.zeros((len(workloads), len(srcs)), dtype='float64')
    parallel_times = np.zeros((len(workloads), len(srcs)), dtype='float64')

    for w_ind, n_ind in product(range(len(workloads)), range(len(srcs))):
        n_pkts = 2 ** exponent
        _log('uniform speedup: n={0}, mean={1}\n'.format(srcs[n_ind], workloads[w_ind]))
        for i in range(n_iters):
            serial_times[w_ind, n_ind] += float(timeout_output(['./firewall', '0', str(n_pkts), str(srcs[n_ind]), str(workloads[w_ind]), '1', str(i), '32'], 2))
            parallel_times[w_ind, n_ind] += float(timeout_output(['./firewall', '2', str(n_pkts), str(srcs[n_ind]), str(workloads[w_ind]), '1', str(i), '32'], 2))

    serial_times /= n_iters
    parallel_times /= n_iters

    np.savetxt('results/unif_speedup/serial_times_{0}.csv'.format(exponent), serial_times, delimiter=',')
    np.savetxt('results/unif_speedup/parallel_times_{0}.csv'.format(exponent), parallel_times, delimiter=',')

def exp_speedup(exponent, upto):
    workloads = [1000, 2000, 4000, 8000]
    srcs = [1, 3, 7, 15, 31, 63][:upto]
    n_iters = 5

    serial_times = np.zeros((len(workloads), len(srcs)), dtype='float64')
    parallel_times = np.zeros((len(workloads), len(srcs)), dtype='float64')

    for w_ind, n_ind in product(range(len(workloads)), range(len(srcs))):
        n_pkts = 2 ** exponent
        _log('exp speedup: n={0}, mean={1}\n'.format(srcs[n_ind], workloads[w_ind]))
        for i in range(n_iters):
            serial_times[w_ind, n_ind] += float(timeout_output(['./firewall', '0', str(n_pkts), str(srcs[n_ind]), str(workloads[w_ind]), '0', str(i), '32'], 2))
            parallel_times[w_ind, n_ind] += float(timeout_output(['./firewall', '2', str(n_pkts), str(srcs[n_ind]), str(workloads[w_ind]), '0', str(i), '32'], 2))

    serial_times /= n_iters
    parallel_times /= n_iters

    np.savetxt('results/exp_speedup/serial_times_{0}.csv'.format(exponent), serial_times, delimiter=',')
    np.savetxt('results/exp_speedup/parallel_times_{0}.csv'.format(exponent), parallel_times, delimiter=',')

def q_depth(exponent):
    n_iters = 3
    workloads = [1, 500, 5000]
    srcs = [1, 3, 7, 15]
    depths = [1, 2, 4, 8, 32, 256]

    runtimes = np.zeros((len(workloads), len(srcs), len(depths)), dtype='float64')
    for w_ind, n_ind, D_ind in product(range(len(workloads)), range(len(srcs)), range(len(depths))):
        n_pkts = 2 ** exponent
        _log('q depth: n={0}, mean={1}, depth={2}\n'.format(srcs[n_ind], workloads[w_ind], depths[D_ind]))
        for i in range(n_iters):
            runtimes[w_ind, n_ind, D_ind] += float(timeout_output(['./firewall', '2', str(n_pkts), str(srcs[n_ind]), str(workloads[w_ind]), '0', str(i), str(depths[D_ind])], 2))

    runtimes /= n_iters

    f = open('results/q_depth.npy', 'w')
    np.save(f, runtimes)
    f.close()

if __name__ == '__main__':
    os.mkdir('results')
    global logfile
    logfile = open('/tmp/hallliu_log', 'w', 0)

    #counter_overhead_work()

    #parallel_overhead(24)
    #dispatcher_rate(20)
    #unif_speedup(14, 5)
    #exp_speedup(14, 5)
    #unif_speedup(10, 7)
    #exp_speedup(10, 7)
    #q_depth(14)
    logfile.close()
