#!/usr/bin/python2
import sys
import subprocess as sp
import unittest
import Queue
import random


class QueueTests(unittest.TestCase):
    def serial_queue_helper(self, q_size):
        proc = sp.Popen(["./test_main", "queue_serial", str(q_size)], stdin=sp.PIPE, stdout=sp.PIPE)
        self.stdin = proc.stdin
        self.stdout = proc.stdout

    '''
    Serial queue tests. Includes some standard cases and some edge cases,
    as well as a random case.
    '''
    def test_immediate_dq(self):
        for size in [1,2,4]:
            self.serial_queue_helper(size)
            self.stdin.write('d\n')
            self.stdin.write('x\n')
            assert (self.stdout.readline().strip() == 'E')

    def test_full_enq(self):
        for size in [1, 2, 4, 8]:
            self.serial_queue_helper(size)
            for i in range(size + 1):
                self.stdin.write('e {0}\n'.format(i))
            self.stdin.write('x\n')
            assert (self.stdout.readline().strip() == 'F')

    def test_fill_and_empty(self):
        for size in [1, 2, 16, 32]:
            self.serial_queue_helper(size)
            for i in range(size):
                self.stdin.write('e {0}\n'.format(i))
            for i in range(size):
                self.stdin.write('d\n')
            self.stdin.write('x\n')
            for i in range(size):
                assert self.stdout.readline().strip() == str(i)

    def test_alternate_fill(self):
        for size in [1, 2, 16, 32]:
            self.serial_queue_helper(size)
            for i in range(size):
                self.stdin.write('e {0}\n'.format(i))
                self.stdin.write('d\n')
                self.stdin.flush()
            self.stdin.write('x\n')
            for i in range(size):
                assert self.stdout.readline().strip() == str(i)

    '''
    Generates a random sequence of enqueues and dequeues, plugs them into
    the builtin Python queue, and makes sure that the output from
    our queue matches.
    '''
    def test_random_fill(self):
        for size in [1, 2, 4, 8, 32, 128]:
            pyQ = Queue.Queue(maxsize=size)
            serial_queue_helper(size)
            expected = []
    
            for i in range(100): # perform 100 actions
                action = random.randint(0, 1)
                if action == 0:
                    to_enq = random.randint(1, 1000)
                    try:
                        self.stdin.write('e {0}\n'.format(to_enq))
                        pyQ.put(to_enq, block=False)
                    except Queue.Full:
                        expected.append('F')
    
                else:
                    try:
                        self.stdin.write('d\n')
                        dqd = pyQ.get(block=False)
                        expected.append(str(dqd))
                    except Queue.Empty:
                        expected.append('E')
            self.stdin.write('x\n')

            for i in expected:
                assert self.stdout.readline().strip() == i


    '''
    Parallel queue tests. Python does very little of the heavy lifting
    here because it involves threading.
    '''
    def test_fast_dqr(self):
        for size in [1, 2, 4, 8, 16, 32]:
            for enq_count in [1, 10, 100, 1000]:
                for delay_mode in [0, 1, 2, 3]:
                    out = sp.check_output(['test_main', 'queue_parallel_1', str(size), str(enq_count), str(delay_mode)])
                    if len(out) > 0:
                        raise AssertionError('{0}\nFailed with size={1}, enq_count={2}, delay={3}'.format(
                            out, size, enq_count, delay_mode))


    def test_fast_eqr(self):
        for size in [1, 2, 4, 8, 16, 32]:
            for enq_count in [1, 10, 100, 1000]:
                for delay_mode in [0, 1, 2, 3]:
                    out = sp.check_output(['test_main', 'queue_parallel_2', str(size), str(enq_count), str(delay_mode)])
                    if len(out) > 0:
                        raise AssertionError('{0}\nFailed with size={1}, enq_count={2}, delay={3}'.format(
                            out, size, enq_count, delay_mode))

