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
    def test_fingerprints(self):

