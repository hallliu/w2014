#!/usr/bin/python2
import os
import subprocess as sp
import unittest

class QueueTests(unittest.TestCase):
    def serial_queue_helper(self, q_size):
        proc = sp.Popen(["test_main", "queue_serial", str(q_size)], stdin=sp.PIPE, stdout=sp.PIPE)
        self.stdin = stdin
        self.stdout = stdout

    def test_immediate_dq(self):
        for size in [1,2,4]:
            self.serial_queue_helper(size)
            self.stdin.write('d\n')
            assert (self.stdout.readline().strip() == 'E')

    def test_full_enq(self):
        for size in [1, 2, 4, 8]:
            self.serial_queue_helper(size)
            for i in range(size + 1):
                self.stdin.write('e {0}\n'.format(i))
            assert (self.stdout.readline().strip() == 'F')

    def tearDown(self):
        self.stdin.write('x\n')
