import numpy as np
import scipy.stats as st
import matplotlib.pyplot as plt
import pickle
from itertools import product

colors = ['black', 'violet', 'blue', 'green', 'yellow', 'orange', 'red']

rhos = [0.5, 0.75, 0.9, 0.99]
srcs = [1, 2, 3, 7, 15, 31]
tables = ['locked', 'lfc', 'probe', 'split']

def reads_speedup():
    parallel_data= np.load('results/reads/parallel.npy')
    serial_data = np.load('results/reads/serial.npy')
    
    for (r_ind, r) in enumerate(rhos):
        plt.figure()
        plots = []
        for ind, l in enumerate(tables):
            plots.append(plt.plot(srcs, parallel_data[r_ind, :, ind]/serial_data[r_ind], color=colors[ind+1], linestyle='-')[0])

        plt.xlim((0, 32))
        plt.ylim((-0.01, 3))
        plt.xscale('log', basex=2)
        plt.legend(plots, tables, loc=0)
        plt.savefig('../img/reads_{0}.png'.format(r), dpi=150, bbox_inches='tight')

def writes_speedup():
    parallel_data= np.load('results/writes/parallel.npy')
    serial_data = np.load('results/reads/serial.npy')
    
    for (r_ind, r) in enumerate(rhos):
        plt.figure()
        plots = []
        for ind, l in enumerate(tables):
            plots.append(plt.plot(srcs, parallel_data[r_ind, :, ind]/serial_data[r_ind], color=colors[ind+1], linestyle='-')[0])

        plt.xlim((0, 32))
        plt.ylim((-0.01, 3))
        plt.xscale('log', basex=2)
        plt.legend(plots, tables, loc=0)
        plt.savefig('../img/writes_{0}.png'.format(r), dpi=150, bbox_inches='tight')
