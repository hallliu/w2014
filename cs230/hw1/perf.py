#!/usr/bin/python2
import wrapper
import numpy as np
import scipy.sparse as sp

sizes = [16,23,64,128,256,512,1024]
threads = [1,2,4,8,16,32,64]

def gen_random_adj(size):
    '''
    Since a randomly generated matrix will probably be more-or-less complete,
    let's start with a sparse matrix
    '''
    rinds, cinds = sp.rand(size, size, format='dok', density=0.1).nonzero()
    randvals = np.random.randint(0, 1001, size=rinds.shape[0])
    adj = 10000000 * np.ones((size, size))
    for i in range(rinds.shape[0]):
        adj[rinds[i], cinds[i]] = randvals[i]

    np.fill_diagonal(adj, 0)

    return adj


def parallel_overhead():
    overheads = {}
    for n in sizes:
        adj = gen_random_adj(n)
        serial_time = wrapper.fw_serial(adj, n)
        parallel_time = wrapper.fw_parallel(adj, n, 1)
        overheads[i] = parallel_time - serial_time

        print parallel_time, serial_time, overheads[i]
    
    return overheads
