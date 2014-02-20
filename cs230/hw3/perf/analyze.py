import numpy as np
import scipy.stats as st
import matplotlib.pyplot as plt
import pickle
from itertools import product

colors = ['black', 'violet', 'blue', 'green', 'yellow', 'orange', 'red']

works = [1000, 2000, 4000, 8000]
srcs = [1, 3, 7, 15]
locks = ['TAS', 'backoff', 'mutex', 'Alock', 'CLH', 'MCS']

def uniform_speedup():
    lockfree_data = np.load('results/unif/lockfree.npy')
    random_data = np.load('results/unif/random.npy')
    lq_data = np.load('results/unif/home.npy')
    
    # Generate 8 graphs...
    '''
    for (w_ind, w) in enumerate(works):
        plt.figure()
        plots = []
        for ind, l in enumerate(locks):
            plots.append(plt.plot(srcs, random_data[w_ind, :, ind], color=colors[ind+1], linestyle='-')[0])
        plots.append(plt.plot(srcs, lockfree_data[w_ind, :], color=colors[0], linestyle='-')[0])

        plt.xlim((0, 16))
        plt.xscale('log', basex=2)
        plt.legend(plots, locks + ['lockfree'], loc=0)
        plt.savefig('../img/rand_{0}.png'.format(w), dpi=150, bbox_inches='tight')

        plt.figure()
        plots = []
        for ind, l in enumerate(locks):
            plots.append(plt.plot(srcs, lq_data[w_ind, :, ind], color=colors[ind+1], linestyle='-')[0])
        plots.append(plt.plot(srcs, lockfree_data[w_ind, :], color=colors[0], linestyle='-')[0])

        plt.xlim((0, 16))
        plt.xscale('log', basex=2)
        plt.legend(plots, locks + ['lockfree'], loc=0)
        plt.savefig('../img/lq_{0}.png'.format(w), dpi=150, bbox_inches='tight')
    '''

def exp_speedup():
    lockfree_data = np.load('results/exp/lockfree.npy')
    random_data = np.load('results/exp/random.npy')
    lq_data = np.load('results/exp/home.npy')
    
    # Generate 8 graphs...
    '''
    for (w_ind, w) in enumerate(works):
        plt.figure()
        plots = []
        for ind, l in enumerate(locks):
            plots.append(plt.plot(srcs, random_data[w_ind, :, ind], color=colors[ind+1], linestyle='-')[0])
        plots.append(plt.plot(srcs, lockfree_data[w_ind, :], color=colors[0], linestyle='-')[0])

        plt.xlim((0, 16))
        plt.xscale('log', basex=2)
        plt.legend(plots, locks + ['lockfree'], loc=0)
        plt.savefig('../img/exp/rand_{0}.png'.format(w), dpi=150, bbox_inches='tight')

        plt.figure()
        plots = []
        for ind, l in enumerate(locks):
            plots.append(plt.plot(srcs, lq_data[w_ind, :, ind], color=colors[ind+1], linestyle='-')[0])
        plots.append(plt.plot(srcs, lockfree_data[w_ind, :], color=colors[0], linestyle='-')[0])

        plt.xlim((0, 16))
        plt.xscale('log', basex=2)
        plt.legend(plots, locks + ['lockfree'], loc=0)
        plt.savefig('../img/exp/lq_{0}.png'.format(w), dpi=150, bbox_inches='tight')
        '''

def packet_overhead():
    data = np.loadtxt('./results/packet_overhead.csv', delimiter=',')
    works = [25, 50, 100, 200, 400, 800]
    data = data.T

    locked_data = data[:-1, :]
    locked_data[:, :2] /= 2 #screwed something up in perf.py....
    import ipdb;ipdb.set_trace()
    lockfree_data = data[-1. :]

    locked_data /= lockfree_data

    plt.figure()
    plots = []
    for ind, l in enumerate(locks):
        plots.append(plt.plot(works, locked_data[ind, :], color=colors[ind], linestyle='-')[0])

    plt.xlim((20, 1000))
    plt.xscale('log', basex=2)
    plt.legend(plots, locks, loc=0)
    plt.savefig('../img/pkt_overhead.png', dpi=150, bbox_inches='tight')

def counter_overhead_time():
    data = np.loadtxt('./results/counter_overhead_time.csv')
    data_locks = data[:-1] / data[-1]
    for i in data_locks:
        print '{:0.3f}&'.format(i),

def counter_overhead_work():
    data = np.loadtxt('./results/counter_overhead_work.csv')
    data_locks = data[:-1] / data[-1]
    for i in data_locks:
        print '{:0.3f}&'.format(i),

def counter_scaling_time():
    data_main = np.loadtxt('./results/counter_scaling_time.csv', delimiter=',') / 3
    no_lock_tp = np.loadtxt('./results/counter_overhead_time.csv', delimiter=',')[-1]
    srcs = [1, 2, 4, 8, 16, 32]#, 64] 

    data_main /= no_lock_tp

    plt.figure()
    plots = []
    for ind, l in enumerate(locks):
        plots.append(plt.plot(srcs, data_main[ind, :], color=colors[ind], linestyle='-')[0])

    plt.xlim((0, 32))
    plt.xscale('log', basex=2)
    plt.xticks(srcs)
    plt.legend(plots, locks, loc=0)
    plt.savefig('../img/counter_scaling_time.png', dpi=150, bbox_inches='tight')

def counter_scaling_work():
    data_main = np.loadtxt('./results/counter_scaling_work.csv', delimiter=',')
    no_lock_tp = np.loadtxt('./results/counter_overhead_work.csv', delimiter=',')[-1]
    srcs = [1, 2, 4, 8, 16, 32]#, 64] 

    data_main /= no_lock_tp

    plt.figure()
    plots = []
    for ind, l in enumerate(locks):
        plots.append(plt.plot(srcs, data_main[ind, :], color=colors[ind], linestyle='-')[0])

    plt.xlim((0, 32))
    plt.xscale('log', basex=2)
    plt.xticks(srcs)
    plt.legend(plots, locks, loc=0)
    plt.savefig('../img/counter_scaling_work.png', dpi=150, bbox_inches='tight')

def counter_fairness():
    tps = np.loadtxt('./results/counter_fairness_tp.csv')
    stds = np.loadtxt('./results/counter_fairness_stds.csv')

    plt.scatter(stds, tps)
    for (i, l) in enumerate(locks):
        plt.annotate(l, xy=(stds[i], tps[i]), xytext=(-40, 0), textcoords = 'offset points',
                arrowprops = {'arrowstyle':'->'})

    plt.savefig('../img/counter_fairness.png', dpi=150, bbox_inches='tight')

def stat_worker():
    stat_tps = np.loadtxt('./results/stat.npy', delimiter=',')
    random_tps = np.load('./results/exp/random.npy')[:2,3,:]
    lq_tps = np.load('./results/exp/home.npy')[:2,3,:]

    return (stat_tps / random_tps, stat_tps / lq_tps)


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



