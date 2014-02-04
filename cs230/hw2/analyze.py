import numpy as np
import scipy.stats as st
import matplotlib.pyplot as plt
import pickle
from itertools import product

colors = ['black', 'violet', 'blue', 'green', 'yellow', 'orange', 'red']

def parallel_overheads():
    srcs = np.array([1, 3, 7, 15])
    means = np.array([25, 50, 100, 200, 400, 800])
    xline = np.linspace(25, 800)

    n_pkts = 2 ** 24 / means

    serial_t = np.loadtxt('results/parallel_overhead/serial_times.csv', delimiter=',')
    serial_q_t = np.loadtxt('results/parallel_overhead/serial_queue_times.csv', delimiter=',')
    speedups = serial_q_t / serial_t

    t_to_runtime_ratio = serial_q_t[0] / n_pkts
    m, yint = st.linregress(means, t_to_runtime_ratio)[:2]
    print 'slope: {0}, yint: {1}'.format(m, yint)
    rangeline = m * xline + yint

    plt.figure()
    plots = []
    names = map(lambda x:'{0:d} sources'.format(x), srcs)
    for ind, src in enumerate(srcs):
        plots.append(plt.plot(means, speedups[ind], color=colors[ind], linestyle='-')[0])

    plt.xlim((10, 1000))
    plt.xscale('log', basex=10)
    plt.legend(plots, names, loc=0)
    plt.savefig('img/parallel_overheads.png', dpi=150, bbox_inches='tight')

    plt.figure()
    plt.scatter(means, t_to_runtime_ratio)
    plt.plot(xline, rangeline)
    #plt.savefig('img/worker_rate.png', dpi=150, bbox_inches='tight')

def dispatch_rate():
    srcs = np.array([1, 2, 3, 4, 7, 9, 13, 15, 31])

    d_rates = np.loadtxt('results/dispatcher_rate.csv', delimiter=',')
    nt_to_runtime = 2**20 / d_rates

    plt.figure()
    plt.plot(srcs, nt_to_runtime)
    plt.savefig('img/dispatcher_rate.png', dpi=150, bbox_inches='tight')

def uniform_speedup_14():
    srcs = np.array([1, 3, 7, 15, 31, 63])
    workloads = np.array([1000, 2000, 4000, 8000])

    serial_times_14 = np.loadtxt('results/unif_speedup/serial_times_14.csv', delimiter=',')[:, :4]
    parallel_times_14 = np.loadtxt('results/unif_speedup/parallel_times_14.csv', delimiter=',')[:, :4]

    d_rates =  (2**20 / np.loadtxt('results/dispatcher_rate.csv', delimiter=','))[np.array([0, 2, 4, 7])]
    w_rates = 1. / (2.618e-6 * workloads)

    expected_serial_runtimes = np.empty((4, 4), dtype='float64')
    expected_parallel_runtimes = np.empty((4, 4), dtype='float64')
    for i, j in product(range(4), range(4)):
        expected_parallel_runtimes[i, j] = 2 ** 17 * srcs[j]/min(w_rates[i] * srcs[j], d_rates[j])
        expected_serial_runtimes[i, j] = 2 ** 17 * srcs[j]/min(w_rates[i], d_rates[0])

    p_speedup = serial_times_14 / parallel_times_14
    expected_p_speedup = expected_serial_runtimes / expected_parallel_runtimes 
    plt.figure()
    plots = []
    names = map(lambda x:'{0:d} workload'.format(x), workloads)
    for ind, w in enumerate(workloads):
        plots.append(plt.plot(srcs[:4], p_speedup[ind], color=colors[ind], linestyle='-')[0])

    plt.legend(plots, names, loc=0)
    plt.savefig('img/parallel_speedup_14.png', dpi=150, bbox_inches='tight')
    '''
    plt.figure()
    plots = []
    names = map(lambda x:'{0:d} workload'.format(x), workloads)
    for ind, w in enumerate(workloads):
        plots.append(plt.plot(srcs[:4], expected_p_speedup[ind], color=colors[ind], linestyle='-')[0])

    plt.legend(plots, names, loc=0)
    plt.savefig('img/exp_parallel_speedup_14.png', dpi=150, bbox_inches='tight')
    '''

def uniform_speedup_10():
    srcs = np.array([1, 3, 7, 15, 31, 63])
    workloads = np.array([1000, 2000, 4000, 8000])

    serial_times_10 = np.loadtxt('results/unif_speedup/serial_times_10.csv', delimiter=',')
    parallel_times_10 = np.loadtxt('results/unif_speedup/parallel_times_10.csv', delimiter=',')


    p_speedup = serial_times_10 / parallel_times_10
    plt.figure()
    plots = []
    names = map(lambda x:'{0:d} workload'.format(x), workloads)
    for ind, w in enumerate(workloads):
        plots.append(plt.plot(srcs, p_speedup[ind], color=colors[ind], linestyle='-')[0])

    plt.legend(plots, names, loc=0)
    plt.savefig('img/parallel_speedup_10.png', dpi=150, bbox_inches='tight')

def exponential_speedup_14():
    srcs = np.array([1, 3, 7, 15, 31, 63])
    workloads = np.array([1000, 2000, 4000, 8000])

    serial_times_14 = np.loadtxt('results/exp_speedup/serial_times_14.csv', delimiter=',')[:, :4]
    parallel_times_14 = np.loadtxt('results/exp_speedup/parallel_times_14.csv', delimiter=',')[:, :4]

    p_speedup = serial_times_14 / parallel_times_14
    plt.figure()
    plots = []
    names = map(lambda x:'{0:d} workload'.format(x), workloads)
    for ind, w in enumerate(workloads):
        plots.append(plt.plot(srcs[:4], p_speedup[ind], color=colors[ind], linestyle='-')[0])

    plt.legend(plots, names, loc=0)
    plt.savefig('img/exp_speedup_14.png', dpi=150, bbox_inches='tight')


def q_depths():
    loads = np.array([ 1, 500, 5000])
    srcs = np.array([1, 3, 7, 15])
    depths = np.array([1, 2, 4, 8, 32, 256])

    data = np.load('results/q_depth.npy')
    for ind, w in enumerate(loads):
        dw = data[ind]
        plt.figure()
        plots = []
        names = map(lambda x:'{0:d} sources'.format(x), srcs)
        for ind1, s in enumerate(srcs):
            plots.append(plt.plot(depths, dw[ind1], color=colors[ind1], linestyle='-')[0])
    
        plt.legend(plots, names, loc=0)
        plt.xscale('log', basex=2)
        plt.savefig('img/q_depth_{0}.png'.format(w), dpi=150, bbox_inches='tight')



