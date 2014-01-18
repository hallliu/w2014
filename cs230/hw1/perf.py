#!/usr/bin/python2
import wrapper
import numpy as np
import scipy.sparse as sp
import cPickle
import os, sys

sizes = [16,32,64,128,256,512,1024]
threads = [1,2,4,8,16,32,64]

def gen_random_adj(size):
    '''
    Since a randomly generated matrix will probably be more-or-less complete,
    let's start with a sparse matrix
    '''
    rinds, cinds = sp.rand(size, size, format='dok', density=0.1).nonzero()
    randvals = np.random.randint(0, 1001, size=rinds.shape[0])
    adj = 10000000 * np.ones((size, size), dtype='int32')
    for i in range(rinds.shape[0]):
        adj[rinds[i], cinds[i]] = randvals[i]

    np.fill_diagonal(adj, 0)

    return adj


def parallel_overhead():
    overheads = {}
    for n in sizes:
        serial_times = np.zeros(3, dtype='float64')
        parallel_times = np.zeros(3, dtype='float64')
        for i in range(3):
            adj = gen_random_adj(n)
            serial_times[i] = wrapper.fw_serial(adj, n)[1]
            parallel_times[i] = wrapper.fw_parallel(adj, n, 1)[1]

        overheads[n] = ((parallel_times - serial_times).mean(), (parallel_times - serial_times).std())
    
    return overheads

def speedups():
    spds = np.zeros((len(sizes), len(threads)), dtype='float64')
    spd_stddevs = np.zeros((len(sizes), len(threads)), dtype='float64')

    for (n_ind, n) in enumerate(sizes):
        this_n_spds = np.zeros((3, len(threads)), dtype='float64')
        for i in range(3):
            adj = gen_random_adj(n)
            serial_time = wrapper.fw_serial(adj, n)[1]
            for (t_ind, t) in enumerate(threads):
                p_time = wrapper.fw_parallel(adj, n, t)[1]
                this_n_spds[i, t_ind] = serial_time / p_time

        spds[n_ind] = this_n_spds.mean(0)
        spd_stddevs[n_ind] = this_n_spds.std(0)

    return spds, spd_stddevs

'''
Note: this should be run with the version compiled with the -DTIME_WAIT compiler flag
'''
def waits():
    waits = np.zeros((len(sizes), len(threads)), dtype='float64')

    for (n_ind, n) in enumerate(sizes):
        adj = gen_random_adj(n)
        for t_ind, t in enumerate(threads):
            waits[n_ind, t_ind] = wrapper.fw_parallel(adj, n, t)[1]

    return waits

def main():
    try:
        os.mkdir('results')
    except:
        pass

    if len(sys.argv) == 1:
        overheads = parallel_overhead()
        spds, spd_stddevs = speedups()
        f = open('results/overheads', 'w')
        cPickle.dump(overheads, f)
        np.savetxt('results/spds.csv', spds, delimiter=',')
        np.savetxt('results/spd_std.csv', spd_stddevs, delimiter=',')

    else:
        w = waits()
        np.savetxt('results/waits.csv', w, delimiter=',')

if __name__ == '__main__':
    main()
