#!/usr/bin/python2
import wrapper
import numpy as np
import os
import unittest

class UtilTests(unittest.TestCase):
    def test_load_csv(self):
        self.filename = '/tmp/TEST_LOAD_CSV_hallliu.csv'
        a = np.random.randint(10000000, size=(40,40))
        b = np.empty((40,40), dtype='int32')
        np.savetxt(self.filename, a, delimiter=',', fmt='%d')
        wrapper.from_csv(self.filename, b, 40)
        self.assertTrue(np.all(a == b))
    
    def test_dump_csv(self):
        self.filename = '/tmp/TEST_DUMP_CSV_hallliu.csv'
        a = np.random.randint(10000000, size=(40,40)).astype('int32')
        wrapper.to_csv(self.filename, a, 40)
        b = np.loadtxt(self.filename, delimiter=',', dtype='int32')
        self.assertTrue(np.all(a == b))
    
    def tearDown(self):
        os.remove(self.filename)

class CorrectnessTests(unittest.TestCase):
    def setUp(self):
        self.sizes = [10, 50, 100]
    '''
    Helper function that actually runs the FW algorithm and raises helpful
    exceptions when things go wrong
    '''
    def verify_expectations(self, expected, adj, name):
        thread_cts = [1, 2, 4, 8, 16, 32]
        serial_result = wrapper.fw_serial(adj, adj.shape[0])
        if not np.all(serial_result == expected):
            raise AssertionError('Serial version failed in {0}'.format(name))

    '''
        failures = []
        for t in thread_cts:
            result = wrapper.fw_parallel(adj, adj.shape[0], t)
            if not np.all(result == expected):
                failures.append(t)

        if len(failures) > 0:
            raise AssertionError('Parallel version failed on T={0} in {1}'.format(failures, name))
    '''
    '''
    Meant to test correct functionality on a hand-verified
    test case (i.e. on a small graph)
    '''
    def test_small_regular_case(self):
        expected_result = np.array([[0, 2, 10000000, 10000000, 5],
                                    [10000000, 0, 10000000, 10000000, 3],
                                    [100, 3, 0, 1, 1],
                                    [10000000, 10000000, 10000000, 0, 10000000],
                                    [10000000, 2, 10000000, 10000000, 0]], dtype='int32')
        adj_matrix = np.array([[0, 2, 10000000, 10000000, 10000000],
                               [10000000, 0, 10000000, 10000000, 3],
                               [100, 10000000, 0, 1, 1],
                               [10000000, 10000000, 10000000, 0, 10000000],
                               [10000000, 2, 10000000, 10000000, 0]], dtype='int32')

        self.verify_expectations(expected_result, adj_matrix, 'small_regular')

    def test_singleton(self):
        expected = np.array([[0]], dtype='int32')
        adj = np.array([[0]], dtype='int32')
        self.verify_expectations(expected, adj, 'singleton')

    def test_pairs(self):
        d00_expected = np.array([[0, 10000000],[10000000, 0]], dtype='int32')
        d00_adj = np.array([[0, 10000000],[10000000, 0]], dtype='int32')

        d02_expected = np.array([[0, 2],[10000000, 0]], dtype='int32')
        d02_adj = np.array([[0, 2],[10000000, 0]], dtype='int32')

        d20_expected = np.array([[0, 10000000],[2, 0]], dtype='int32')
        d20_adj = np.array([[0, 10000000],[2, 0]], dtype='int32')

        d22_expected = np.array([[0, 2],[2, 0]], dtype='int32')
        d22_adj = np.array([[0, 2],[2, 0]], dtype='int32')

        self.verify_expectations(d00_expected, d00_adj, 'd00')
        self.verify_expectations(d02_expected, d02_adj, 'd02')
        self.verify_expectations(d20_expected, d20_adj, 'd20')
        self.verify_expectations(d22_expected, d22_adj, 'd22')

    def test_empty(self):
        adjs = {}
        expecteds = {}
        for i in self.sizes:
            adjs[i] = 10000000 * np.ones((i, i), dtype='int32')
            np.fill_diagonal(adjs[i], 0)
            expecteds[i] = adjs[i].copy()
            self.verify_expectations(expecteds[i], adjs[i], 'empty{0}'.format(i))

    def test_one_edge(self):
        adjs = {}
        expecteds = {}
        for i in self.sizes:
            adjs[i] = 10000000 * np.ones((i, i), dtype='int32')
            np.fill_diagonal(adjs[i], 0)

            random_edge = (0,0)
            while random_edge[0] == random_edge[1]:
                random_edge = np.random.randint(i, size=(2,))

            adjs[i][random_edge[0], random_edge[1]] = 1
            expecteds[i] = adjs[i].copy()
            self.verify_expectations(expecteds[i], adjs[i], 'one_edge{0}'.format(i))

    def test_complete(self):
        adjs = {}
        expecteds = {}
        for i in self.sizes:
            adjs[i] = np.ones((i, i), dtype='int32')
            np.fill_diagonal(adjs[i], 0)
            expecteds[i] = adjs[i].copy()
            self.verify_expectations(expecteds[i], adjs[i], 'complete{0}'.format(i))

    def test_linear(self):
        for i in range(2):
            graph_size = np.random.randint(2, 512)
            seq = np.random.permutation(graph_size)
            adj = np.ones((graph_size, graph_size), dtype='int32') * 10000000
            np.fill_diagonal(adj, 0)

            # Fill in the edges
            for i in range(graph_size - 1):
                adj[seq[i], seq[i+1]] = 1
                adj[seq[i+1], seq[i]] = 1

            seq_inv = np.empty(graph_size, dtype='int32')
            for i in range(graph_size):
                seq_inv[i] = np.where(seq == i)[0][0]

            expected = np.zeros((graph_size, graph_size), dtype='int32')
            for i in range(graph_size):
                for j in range(graph_size):
                    expected[i, j] = abs(seq_inv[i] - seq_inv[j])

            self.verify_expectations(expected, adj, 'linear{0}'.format(graph_size))
             
if __name__ == '__main__':
    unittest.main()
