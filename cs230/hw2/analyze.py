import numpy as np
import scipy.stats as st
import matplotlib.pyplot as plt
import pickle

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

def analyze_runtimes():
    threads = np.exp2(np.arange(7))
    spds = np.loadtxt('results/spds.csv', delimiter=',')
    spd_stds = np.loadtxt('results/spd_std.csv', delimiter=',')

    colors = ['black', 'violet', 'blue', 'green', 'yellow', 'orange', 'red']
    color_names = map(lambda x: '{0:d} Vertices'.format(int(x)), np.exp2(np.arange(4,11)))
    plt.figure(figsize=(12, 8))
    plots = []
    for i in range(spds.shape[0]):
        print colors[i], max(spds[i])
        #plt.plot(threads, spds[i], color=colors[i], linestyle='-')
        plots.append(plt.errorbar(threads, spds[i], yerr=spd_stds[i], marker=None, ecolor=colors[i], color=colors[i])[0])

    plt.xscale('log', basex=2)
    plt.legend(plots, color_names, loc=1)
    plt.savefig('img/speedups.png', dpi=200, bbox_inches='tight')

def analyze_tdiffs():
    threads = np.exp2(np.arange(7))
    spds = np.loadtxt('results/outerloop_avgs.csv', delimiter=',')
    spd_stds = np.loadtxt('results/outerloop_stds.csv', delimiter=',')

    colors = ['black', 'violet', 'blue', 'green', 'yellow', 'orange', 'red']
    color_names = map(lambda x: '{0:d} Vertices'.format(int(x)), np.exp2(np.arange(4,11)))
    plt.figure(figsize=(12, 8))
    plots = []
    for i in range(spds.shape[0]):
        print colors[i], max(spds[i])
        #plt.plot(threads, spds[i], color=colors[i], linestyle='-')
        plots.append(plt.errorbar(threads, spds[i], yerr=spd_stds[i], marker=None, ecolor=colors[i], color=colors[i])[0])

    plt.xscale('log', basex=2)
    plt.legend(plots, color_names, loc=1)
    plt.savefig('img/outerloop_data.png', dpi=200, bbox_inches='tight')


