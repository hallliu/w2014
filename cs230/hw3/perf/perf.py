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
            out = timeout_output(['./perf_main', 'parallel_time', l, '2000', '1'], 3000)
            lock_tps[i] += float(out.split()[-1])
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

def counter_scaling_time():
    num_threads = [1, 2, 4, 8, 16, 32]#, 64] 
    lock_tps = np.zeros((len(locks), len(num_threads)), dtype='float64')

    for (i, j) in product(range(len(locks)), range(len(num_threads))):
        for k in range(3):
            print('counter_scaling_time: lock {0} threads {1} iteration {2}'.format(locks[i], num_threads[j], k))
            out = timeout_output(['./perf_main', 'parallel_time', locks[i], '2000', str(num_threads[j])], 4)
            lock_tps[i, j] += float(out.split()[-1])

    lock_tps /= 2000
    lock_tps /= 3
    return lock_tps
            
def counter_scaling_work():
    num_threads = [1, 2, 4, 8, 16, 32]#, 64] 
    count_tos = [10000000, 2000000, 1000000, 1000000, 1000000, 10000]#, 10000]
    lock_tps = np.zeros((len(locks), len(num_threads)), dtype='float64')

    for (i, j) in product(range(len(locks)), range(len(num_threads))):
        for k in range(3):
            print('counter_scaling_work: lock {0} threads {1} iteration {2}'.format(locks[i], num_threads[j], k))
            lock_tps[i, j] = count_tos[j] /  float(timeout_output(['./perf_main', 'parallel_work', locks[i], str(count_tos[j]), str(num_threads[j])], 2))

    lock_tps /= 3
    return lock_tps

def counter_fairness():
    num_threads = 4 
    lock_tps = np.zeros(len(locks), dtype='float64')
    lock_stddevs = np.zeros(len(locks), dtype='float64')

    for (i, l) in enumerate(locks):
        for j in range(3):
            print('counter_fairness: lock {0} iteration {1}'.format(l, j))
            outs = timeout_output(['./perf_main', 'parallel_time', l, '2000', str(num_threads)], 3000).split()
            lock_tps[i] += float(outs[-1])
            lock_stddevs[i] += np.std(map(float, outs[:-1]))
    
    lock_tps /= 3
    lock_stddevs /= 3
    return (lock_tps, lock_stddevs)


if __name__ == '__main__':
    os.mkdir('results')
    global logfile
    logfile = open('/tmp/hallliu_log', 'w', 0)

    counter_overhead_work()
    counter_overhead_time()

    counter_scaling_time()
    counter_scaling_work()

    counter_fairness()
    logfile.close()
