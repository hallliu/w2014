#!/usr/bin/python2
import subprocess as sp
import numpy as np
import unittest
import itertools
import Queue
import random
import threading
from subprocess import Popen,PIPE

class RunSP(threading.Thread):
    def __init__(self, cmd, timeout):
        threading.Thread.__init__(self)
        self.timeout = timeout
        self.cmd = cmd

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
    out = RunSP(cmd, timeout).Run()
    if out == 'TIMEOUT':
        return '-1000000'
    else:
        return out

class LockTests(unittest.TestCase):
    def setUp(self):
        self.regular_locks = ['TAS', 'mutex', 'CLH', 'MCS', 'Alock']
        self.delay_tests = [(10, 50000), (20, 25000), (100, 1000), (500, 500)]

    def test_hold_lock(self):
        '''
        First do the non-parametric locks
        '''
        for lock in self.regular_locks:
            for workers in 2**np.arange(5):
                for test_time in 5000*(2**np.arange(4)):
                    cmdstr = ['./test_main', 'hold_lock', lock, str(workers), str(test_time)]
                    print cmdstr
                    out = sp.check_output(cmdstr)
                    if len(out) > 0:
                        raise AssertionError('Failed with lock {0} with {1} workers at {2}us'.format(lock, workers, test_time))

        '''
        Now backoff. 
        '''
        for workers in 2**np.arange(5):
            for test_time in 5000*(2**np.arange(4)):
                for (min_delay, max_delay) in self.delay_tests:
                    cmdstr = ['./test_main', 'hold_lock', 'backoff', str(min_delay), str(max_delay), str(workers), str(test_time)]
                    out = sp.check_output(cmdstr)
                    if len(out) > 0:
                        raise AssertionError('Failed on backoff at {0} with {1} workers at {2}us '.format((min_delay, max_delay), workers, test_time))


    def test_incrementing(self):
        for lock in self.regular_locks:
            for workers in 2**np.arange(5):
                for counter_val in [1, 10, 100, 1000]:
                    cmdstr = ['./test_main', 'incrementing', lock, str(workers), str(counter_val)]
                    print cmdstr
                    out = sp.check_output(cmdstr)
                    if len(out) > 0:
                        raise AssertionError('Failed with lock {0} with {1} workers at {2}us'.format(lock, workers, counter_val))

        for workers in 2**np.arange(5):
            for counter_val in [1, 10, 100, 1000]:
                for (min_delay, max_delay) in self.delay_tests:
                    cmdstr = ['./test_main', 'incrementing', 'backoff', str(min_delay), str(max_delay), str(workers), str(counter_val)]
                    out = sp.check_output(cmdstr)
                    if len(out) > 0:
                        raise AssertionError('Failed on backoff at {0} with {1} workers at {2}us '.format((min_delay, max_delay), workers, counter_val))


    def test_lock_nohang(self):
        '''
        First do the non-parametric locks
        '''
        for lock in self.regular_locks:
            for test_time in 100000*(2**np.arange(4)):
                cmdstr = ['./test_main', 'lock_nohang', lock, str(test_time)]
                print cmdstr
                out = timeout_output(cmdstr, test_time * 1.5)
                if len(out) > 0:
                    raise AssertionError('Failed with lock {0} with at {1}us'.format(lock, test_time))

        '''
        Now backoff. 
        '''
        for workers in 2**np.arange(5):
            for (min_delay, max_delay) in self.delay_tests:
                cmdstr = ['./test_main', 'lock_nohang', 'backoff', str(min_delay), str(max_delay), str(workers), str(test_time)]
                out = timeout_output(cmdstr, test_time * 1.5)
                if len(out) > 0:
                    raise AssertionError('Failed on backoff at {0} at {2}us '.format((min_delay, max_delay), test_time))
        
    def test_ordering(self):
        for lock in ['Alock', 'CLH', 'MCS']:
            for workers in 2**np.arange(5):
                cmdstr = ['./test_main', 'ordering', lock, str(workers)]
                print cmdstr
                out = map(int, sp.check_output(cmdstr).split())

                if not np.all(np.array(out) == np.arange(workers)):
                    raise AssertionError('Failed with lock {0} with {1} workers with ordering {2}'.format(lock, workers, out))

class WorkerTests(unittest.TestCase):
    def setUp(self):
        self.pkts = [1, 2, 4, 32, 1024 ]
        self.srcs = [1, 2, 4, 5]
        self.worker_types = [0, 1, 2]
        self.worker_names = {0: 'homequeue', 1: 'random', 2: 'lastqueue'}
        self.means = [1, 1000]
        self.distrs = [0, 1]
        self.lock_types = ['TAS', 'backoff', 'Alock', 'mutex', 'CLH', 'MCS']

    def test_fingerprints(self):
        for (pkts, srcs, w_type, mean, distr, l_type) in itertools.product(
                self.pkts, self.srcs, self.worker_types, self.means, self.distrs, self.lock_types):
            if l_type == 'backoff':
                cmdstr = ['./test_main', 'general_worker', str(pkts), str(srcs), str(w_type), str(mean), '0', str(distr), 
                        l_type, '10', '1000']
            else:
                cmdstr = ['./test_main', 'general_worker', str(pkts), str(srcs), str(w_type), str(mean), '0', str(distr), 
                        l_type]

            print cmdstr
            out = sp.check_output(cmdstr)
            fp = int(out.split()[-1])

            serial_cmdstr = ['../serial_firewall', str(pkts), str(srcs), str(mean), str(distr), '0']
            serial_fp = int(sp.check_output(serial_cmdstr))
            
            if fp != serial_fp:
                raise AssertionError('Fingerprint incorrect at {0}'.format((pkts, srcs, w_type, mean, distr, l_type)))

    def test_random_distrs(self):
        for (pkts, srcs, mean, distr, l_type) in itertools.product(
                [100, 200, 500], self.srcs, self.means, self.distrs, self.lock_types):

            if l_type == 'backoff':
                cmdstr = ['./test_main', 'general_worker', str(pkts), str(srcs), '1', str(mean), '0', str(distr), 
                        l_type, '10', '1000']
            else:
                cmdstr = ['./test_main', 'general_worker', str(pkts), str(srcs), '1', str(mean), '0', str(distr), 
                        l_type]
 
            print cmdstr
            out = sp.check_output(cmdstr)
            counts = out.split('\n')[:-1]
            for c in counts:
                if np.matrix(c).std() > pkts/srcs:
                    raise AssertionError('Stddev too high at {0}: data is {1}'.format((pkts, srcs, mean, distr, l_type), c))

    def test_lastqueue_order(self):
        for (pkts, srcs) in itertools.product([100, 200, 400], [1, 2, 4, 8, 16]):
            out = sp.check_output(['./test_main', 'lastqueue', str(pkts), str(srcs)]).split('\n')[:-1]
            no_rpts = []
            for l in out:
                if l not in no_rpts:
                    no_rpts.append(l)
            def kf(y):
                x = y.split()
                if x[1] == 'started':
                    return int(x[0])
                else:
                    return -int(x[0])
            no_rpts = np.array(map(kf, no_rpts))

            if not np.all(no_rpts[0::2] == -no_rpts[1::2]):
                raise AssertionError('Out-of-order at {0}'.format((pkts, srcs)))
