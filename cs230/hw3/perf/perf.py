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
    logfile.write('{0}      {1}\n'.format(datetime.datetime.utcnow(), msg))

def counter_overhead_time():
    locks_here = locks + ['noop']

    lock_tps = np.zeros(len(locks_here), dtype='float64')
    for (i, l) in enumerate(locks_here):
        for j in range(4):
            _log('counter_overhead_time: lock {0} iteration {1}'.format(l, j))
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
            _log('counter_overhead_work: lock {0} iteration {1}'.format(l, j))
            lock_tps[i] = count_to / float(timeout_output(['./perf_main', 'parallel_work', l, str(count_to), '1'], 3000))

    lock_tps /= 4
    np.savetxt('results/counter_overhead_time.csv', lock_tps, delimiter=',')
    return lock_tps

def counter_scaling_time():
    num_threads = [1, 2, 4, 8, 16, 32]#, 64] 
    lock_tps = np.zeros((len(locks), len(num_threads)), dtype='float64')

    for (i, j) in product(range(len(locks)), range(len(num_threads))):
        for k in range(3):
            _log('counter_scaling_time: lock {0} threads {1} iteration {2}'.format(locks[i], num_threads[j], k))
            out = timeout_output(['./perf_main', 'parallel_time', locks[i], '2000', str(num_threads[j])], 4)
            lock_tps[i, j] += float(out.split()[-1])

    lock_tps /= 2000
    lock_tps /= 3
    np.savetxt('results/counter_scaling_time.csv', lock_tps, delimiter=',')
    return lock_tps
            
def counter_scaling_work():
    num_threads = [1, 2, 4, 8, 16, 32]#, 64] 
    count_tos = [10000000, 2000000, 1000000, 1000000, 1000000, 10000]#, 10000]
    lock_tps = np.zeros((len(locks), len(num_threads)), dtype='float64')

    for (i, j) in product(range(len(locks)), range(len(num_threads))):
        for k in range(3):
            _log('counter_scaling_work: lock {0} threads {1} iteration {2}'.format(locks[i], num_threads[j], k))
            lock_tps[i, j] = count_tos[j] /  float(timeout_output(['./perf_main', 'parallel_work', locks[i], str(count_tos[j]), str(num_threads[j])], 2))

    lock_tps /= 3
    np.savetxt('results/counter_scaling_work.csv', lock_tps, delimiter=',')
    return lock_tps

def counter_fairness():
    num_threads = 16
    lock_tps = np.zeros(len(locks), dtype='float64')
    lock_stddevs = np.zeros(len(locks), dtype='float64')

    for (i, l) in enumerate(locks):
        for j in range(3):
            _log('counter_fairness: lock {0} iteration {1}'.format(l, j))
            outs = timeout_output(['./perf_main', 'parallel_time', l, '2000', str(num_threads)], 3000).split()
            lock_tps[i] += float(outs[-1]) / 2000
            lock_stddevs[i] += np.std(map(lambda x: float(x) / 2000, outs[:-1]))
    
    lock_tps /= 3
    lock_stddevs /= 3
    np.savetxt('results/counter_fairness_tp.csv', lock_tps, delimiter=',')
    np.savetxt('results/counter_fairness_stds.csv', lock_stddevs, delimiter=',')
    return (lock_tps, lock_stddevs)

def packet_overhead():
    works = [25, 50, 100, 200, 400, 800]
    locks_here = locks + ['noop']

    packet_tps = np.zeros((len(works), len(locks_here)), dtype='float64')

    for (w_ind, l_ind) in product(range(len(works)), range(len(locks_here))):
        for rep in range(3):
            _log('packet_overhead: work {0} lock {1} rep {2}'.format(works[w_ind], locks_here[l_ind], rep))
            out = timeout_output(['./perf_main', 'parallel_dispatcher', '2000', '1', '0', str(works[w_ind]), str(rep), '1', locks_here[l_ind]], 4)
            packet_tps[w_ind, l_ind] += float(out) / 2000

    packet_tps /= 3
    np.savetxt('results/packet_overhead.csv', packet_tps, delimiter=',')
    return packet_tps

def uniform_speedup():
    works = [1000, 2000, 4000, 8000]
    srcs = [1, 3, 7, 15]
    n_reps = 3

    lockfree_tps = np.zeros((len(works), len(srcs)), dtype='float64')
    for (w_ind, s_ind) in product(range(len(works)), range(len(srcs))):
        for rep in range(n_reps):
            _log('uniform speedup lockfree work {0} source {1} rep {2}'.format(works[w_ind], srcs[s_ind], rep))
            out = timeout_output(['./perf_main', 'parallel_dispatcher', '1500', str(srcs[s_ind]), '0', str(works[w_ind]), str(rep), '1', 'noop'], 4)
            lockfree_tps[w_ind, s_ind] += float(out) / 1500

    lockfree_tps /= n_reps

    random_tps = np.zeros((len(works), len(srcs), len(locks)), dtype='float64')
    home_tps = np.zeros((len(works), len(srcs), len(locks)), dtype='float64')

    for (w_ind, s_ind, l_ind) in product(range(len(works)), range(len(srcs)), range(len(locks))):
        for rep in range(n_reps):
            _log('uniform speedup random work {0} source {1} lock {2} rep {3}'.format(works[w_ind], srcs[s_ind], locks[l_ind], rep))
            out = timeout_output(['./perf_main', 'parallel_dispatcher', '1500', str(srcs[s_ind]), '1', str(works[w_ind]), str(rep), '1', locks[l_ind]], 4)
            random_tps[w_ind, s_ind, l_ind] += float(out) / 1500

            _log('uniform speedup homequeue work {0} source {1} lock {2} rep {3}'.format(works[w_ind], srcs[s_ind], locks[l_ind], rep))
            out = timeout_output(['./perf_main', 'parallel_dispatcher', '1500', str(srcs[s_ind]), '2', str(works[w_ind]), str(rep), '1', locks[l_ind]], 4)
            home_tps[w_ind, s_ind, l_ind] += float(out) / 1500

    random_tps /= n_reps
    home_tps /= n_reps

    np.save('results/unif/lockfree', lockfree_tps)
    np.save('results/unif/random', random_tps)
    np.save('results/unif/home', home_tps)
    return (lockfree_tps, random_tps, home_tps)

def exp_speedup():
    works = [1000, 2000, 4000, 8000]
    srcs = [1, 3, 7, 15]
    n_reps = 3

    lockfree_tps = np.zeros((len(works), len(srcs)), dtype='float64')
    for (w_ind, s_ind) in product(range(len(works)), range(len(srcs))):
        for rep in range(n_reps):
            _log('exp speedup lockfree work {0} source {1} rep {2}'.format(works[w_ind], srcs[s_ind], rep))
            out = timeout_output(['./perf_main', 'parallel_dispatcher', '1500', str(srcs[s_ind]), '0', str(works[w_ind]), str(rep), '0', 'noop'], 4)
            lockfree_tps[w_ind, s_ind] += float(out) / 1500

    lockfree_tps /= n_reps

    random_tps = np.zeros((len(works), len(srcs), len(locks)), dtype='float64')
    home_tps = np.zeros((len(works), len(srcs), len(locks)), dtype='float64')

    for (w_ind, s_ind, l_ind) in product(range(len(works)), range(len(srcs)), range(len(locks))):
        for rep in range(n_reps):
            _log('exp speedup random work {0} source {1} lock {2} rep {3}'.format(works[w_ind], srcs[s_ind], locks[l_ind], rep))
            out = timeout_output(['./perf_main', 'parallel_dispatcher', '1500', str(srcs[s_ind]), '1', str(works[w_ind]), str(rep), '0', locks[l_ind]], 4)
            random_tps[w_ind, s_ind, l_ind] += float(out) / 1500

            _log('exp speedup homequeue work {0} source {1} lock {2} rep {3}'.format(works[w_ind], srcs[s_ind], locks[l_ind], rep))
            out = timeout_output(['./perf_main', 'parallel_dispatcher', '1500', str(srcs[s_ind]), '2', str(works[w_ind]), str(rep), '0', locks[l_ind]], 4)
            home_tps[w_ind, s_ind, l_ind] += float(out) / 1500

    random_tps /= n_reps
    home_tps /= n_reps

    np.save('results/exp/lockfree', lockfree_tps)
    np.save('results/exp/random', random_tps)
    np.save('results/exp/home', home_tps)
    return (lockfree_tps, random_tps, home_tps)

def new_workers():
    works = [1000, 2000, 4000, 8000]
    srcs = [3]#, 15]
    n_reps = 2

    stat_tps = np.zeros((len(works), len(srcs), len(locks)), dtype='float64')

    for (w_ind, s_ind, l_ind) in product(range(len(works)), range(len(srcs)), range(len(locks))):
        for rep in range(n_reps):
            #print('exp speedup stat work {0} source {1} lock {2} rep {3}'.format(works[w_ind], srcs[s_ind], locks[l_ind], rep))
            out = timeout_output(['./perf_main', 'parallel_dispatcher', '1000', str(srcs[s_ind]), '2', str(works[w_ind]), str(rep), '0', locks[l_ind]], 4)
            stat_tps[w_ind, s_ind, l_ind] += float(out) / 1000

    stat_tps /= n_reps

    #np.save('results/exp/stat', stat_tps)
    return stat_tps

if __name__ == '__main__':
    os.mkdir('results')
    os.mkdir('results/exp')
    os.mkdir('results/unif')

    global logfile
    logfile = open('/tmp/hallliu_log', 'w', 0)

    '''
    counter_overhead_work()
    counter_overhead_time()

    counter_scaling_time()
    counter_scaling_work()
    '''

    counter_fairness()
    packet_overhead()
    uniform_speedup()
    exp_speedup()
    
    logfile.close()
