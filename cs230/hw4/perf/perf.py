#!/usr/bin/python2
import numpy as np
import matplotlib.pyplot as plt
from subprocess import Popen, PIPE
from itertools import product
import os
import threading
import datetime

logfile = None
tables = ['locked', 'lfc', 'probe', 'split']
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


def timeout_output(cmd, timeout):
    try:
        out = RunSP(cmd, timeout).Run()
    except:
        out = 'TIMEOUT'

    if out == 'TIMEOUT':
        return '-1000000'
    else:
        return out

def _log(msg):
    logfile.write('{0}      {1}\n'.format(datetime.datetime.utcnow(), msg))

def dispatcher_rate():
    iters = 5
    n = 3 #15
    dr = 0

    for i in range(iters):
        _log('dispatcher rate iteration {0}'.format(i))
        out = timeout_output(['./perf_main', 'parallel_noload', '2000', str(n), '0.1', '0.1', '0.1', '128', '0'], 4000)
        dr += float(out.split()[-1]) / 2000
        
    np.savetxt('results/dr.txt', np.array([dr]))
    return dr / iters

def reads_speedup():
    rhos = [0.5, 0.75, 0.9, 0.99]
    ns = [1, 2, 3, 7, 15, 31]
    iters = 4
    results = np.zeros((len(rhos), len(ns), len(tables)), dtype='float64');
    for (r_ind, rho), (n_ind, n), (t_ind, tab) in product(enumerate(rhos), enumerate(ns), enumerate(tables)):
        for i in range(iters):
            _log('read speedup: rho={0} n={1} table {2} iter {3}'.format(rho, n, tab, i))
            out = timeout_output(['./perf_main', 'parallel', '2000', str(n), '0.09', '0.01', str(rho), '128', '500', '0', tab], 3000)
            results[r_ind, n_ind, t_ind] += float(out.split()[-1]) / 2000

    results /= iters
    
    # Now do the serial stuff
    serial_results = np.zeros(len(rhos), dtype='float64')
    for (r_ind, rho) in enumerate(rhos):
        for i in range(iters):
            _log('read serial results: rho={0} iter {1}'.format(rho, i))
            out = timeout_output(['./perf_main', 'serial', '2000', '0.09', '0.01', str(rho), '4', '500', '128'], 3000)
            serial_results[r_ind] += float(out.split()[-1]) / 2000

    serial_results /= iters
    np.save('results/reads/parallel', results)
    np.save('results/reads/serial', serial_results)

    return (serial_results, results)

def writes_speedup():
    rhos = [0.5, 0.75, 0.9, 0.99]
    ns = [1, 2, 3, 7, 15, 31]
    iters = 4
    results = np.zeros((len(rhos), len(ns), len(tables)), dtype='float64');
    for (r_ind, rho), (n_ind, n), (t_ind, tab) in product(enumerate(rhos), enumerate(ns), enumerate(tables)):
        for i in range(iters):
            _log('read speedup: rho={0} n={1} table {2} iter {3}'.format(rho, n, tab, i))
            out = timeout_output(['./perf_main', 'parallel', '2000', str(n), '0.45', '0.05', str(rho), '128', '500', '0', tab], 3000)
            results[r_ind, n_ind, t_ind] += float(out.split()[-1]) / 2000

    results /= iters
    
    # Now do the serial stuff
    serial_results = np.zeros(len(rhos), dtype='float64')
    for (r_ind, rho) in enumerate(rhos):
        for i in range(iters):
            _log('write serial results: rho={0} iter {1}'.format(rho, i))
            out = timeout_output(['./perf_main', 'serial', '2000', '0.45', '0.05', str(rho), '4', '500', '128'], 3000)
            serial_results[r_ind] += float(out.split()[-1]) / 2000

    serial_results /= iters
    np.save('results/writes/parallel', results)
    np.save('results/writes/serial', serial_results)
    return (serial_results, results)
    
if __name__ == '__main__':
    os.mkdir('results')
    os.mkdir('results/reads')
    os.mkdir('results/writes')

    global logfile
    logfile = open('/tmp/hallliu_log', 'w', 0)

    dispatcher_rate()
    reads_speedup()
    writes_speedup()
    logfile.close()
