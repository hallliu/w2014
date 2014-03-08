#!/usr/bin/python2
import subprocess as sp
import numpy as np
import unittest
from itertools import product 
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

class ListTests(unittest.TestCase):
    def test_serial_contains(self):
        for N in [1,2,4,100]:
            cmdstr = ['./test_lists', 'serial_contains', str(N)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}'.format(N))

    def test_serial_removals(self):
        for N in [1,2,4,100]:
            cmdstr = ['./test_lists', 'serial_removals', str(N)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}'.format(N))

    def test_serial_nocontains(self):
        for N in [1,2,4,100]:
            cmdstr = ['./test_lists', 'serial_nocontains', str(N)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}'.format(N))

    def test_serial_ordering(self):
        for N in [1,2,4,100]:
            cmdstr = ['./test_lists', 'serial_ordering', str(N)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}'.format(N))

    def test_serial_empties(self):
        for N in [1,2,4,100]:
            cmdstr = ['./test_lists', 'serial_empties', str(N)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}'.format(N))

    def test_parallel_addcontain1(self):
        for N, T in product([1,2,4,100,2000], [1,2,4,8,16]):
            if T > N:
                continue
            cmdstr = ['./test_lists', 'parallel_addcontain1', str(N), str(T)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}, T={1}'.format(N, T))

    def test_parallel_addcontain2(self):
        for N, T in product([1,2,4,100,2000], [1,2,4,8,16]):
            if T > N:
                continue
            cmdstr = ['./test_lists', 'parallel_addcontain2', str(N), str(T)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}, T={1}'.format(N, T))

    def test_parallel_alltogether(self):
        for N, (Tc, Ta, Tr) in product([1,2,4,100,2000], [(1,2,1),(2,1,1),(1,1,2),(2,4,2),(2,2,4),(4,2,2),(5,1,2),(2,5,1)]):
            if Ta+Tc+Tr > N:
                continue
            cmdstr = ['./test_lists', 'parallel_alltogether', str(N), str(Tc), str(Ta), str(Tr)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}, Tc={1}, Ta={2}, Tr={3}'.format(N, Tc, Ta, Tr))


    def test_parallel_indistinct_add(self):
        for N, T, R in product([1,2,4,100,2000], [1,2,4,8,16], [1,2,4,8,16]):
            if T > N or R > N:
                continue
            cmdstr = ['./test_lists', 'parallel_indistinct_add', str(N), str(T), str(R)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}, T={1}, R={2}'.format(N, T, R))

    def test_parallel_ordering(self):
        for N, T in product([1,2,4,100,2000], [1,2,4,8,16]):
            if T > N:
                continue
            cmdstr = ['./test_lists', 'parallel_ordering', str(N), str(T)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with N={0}, T={1}'.format(N, T))

    def test_parallel_empties(self):
        for T in [1,2,4,8,16]:
            cmdstr = ['./test_lists', 'parallel_empties',  str(T)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with T={0}'.format(T))

class HashTableTests(unittest.TestCase):
    def setUp(self):
        self.tablenames = ['lfc', 'locked']

    def test_addcontain1(self):
        for tab, N, T in product(self.tablenames, [1, 10, 100, 1000, 2000], [1,2,4,8]):
            if T > N:
                continue
            cmdstr = ['./test_lists', 'addcontain1', tab, str(N), str(T)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with table {0}, N={1}, T={2}'.format(tab, N, T))

    def test_addcontain2(self):
        for tab, N, T in product(self.tablenames, [1, 10, 100, 1000, 2000], [1,2,4,8]):
            if T > N:
                continue
            cmdstr = ['./test_lists', 'addcontain2', tab, str(N), str(T)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with table {0}, N={1}, T={2}'.format(tab, N, T))

    def test_alltogether(self):
        for tab, N, (Tc, Ta, Tr) in product(self.tablenames, [10, 100, 1000, 2000],[(1,2,1),(2,1,1),(1,1,2),(2,4,2),(2,2,4),(4,2,2),(5,1,2),(2,5,1)]):
            if (Ta+Tc+Tr) > N:
                continue
            cmdstr = ['./test_lists', 'alltogether', tab, str(N), str(Tc), str(Ta), str(Tr)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with table {0}, N={1}, Tc={2}, Ta={3}, Tr={4}'.format(tab, N, Tc, Ta, Tr))

    def test_indistinct_add(self):
        for tab, N, T, R in product(self.tablenames, [1, 10, 100, 1000, 2000], [1,2,4,8], [1,2,8,16]):
            if T > N or R > N:
                continue
            cmdstr = ['./test_lists', 'indistinct_add', tab, str(N), str(T), str(R)]
            print cmdstr
            out = sp.check_output(cmdstr)
            if len(out) > 0:
                raise AssertionError('Failed with table {0}, N={1}, T={2}, R={3}'.format(tab, N, T, R))
