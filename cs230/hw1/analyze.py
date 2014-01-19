import numpy as np
import scipy.stats as st
import matplotlib.pyplot as plt
import pickle

def analyze_overheads():
    ovs = pickle.load(open('results/overheads', 'r'))
    xvals = np.sort(np.array(ovs.keys()))
    yvals = np.zeros(xvals.shape, dtype='float64')
    yerrs = np.zeros(xvals.shape, dtype='float64')

    for i, x in enumerate(xvals):
        yvals[i] = ovs[x][0]
        yerrs[i] = ovs[x][1]

    reg = st.linregress(np.log2(xvals), np.log2(yvals))
    xline = np.linspace(3, 10.2, 100)
    yline = reg[0]*xline + reg[1]
    print reg[0]

    plt.figure()
    plt.errorbar(xvals, yvals, yerr=yerrs, fmt='.')
    plt.plot(np.exp2(xline), np.exp2(yline), 'k-')
    plt.xlim((10, 1500))
    plt.xscale('log', basex=2)
    plt.yscale('log', basey=2)
    plt.savefig('img/overheads.png', dpi=130, bbox_inches='tight')

def analyze_runtimes():
    threads = np.exp2(np.arange(7))
    spds = np.loadtxt('results/spds.csv', delimiter=',')
    spd_stds = np.loadtxt('results/spd_std.csv', delimiter=',')

    colors = ['black', 'violet', 'blue', 'green', 'yellow', 'orange', 'red']
    plt.figure(figsize=(8, 5))
    for i in range(spds.shape[0]):
        print colors[i], max(spds[i])
        #plt.plot(threads, spds[i], color=colors[i], linestyle='-')
        plt.errorbar(threads, spds[i], yerr=spd_stds[i], marker=None, ecolor=colors[i], color=colors[i])

    plt.xscale('log', basex=2)
    plt.savefig('img/speedups.png', dpi=200, bbox_inches='tight')



