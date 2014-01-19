#!/usr/bin/python2
from distutils.core import setup, Extension
import sys
import numpy as np

compile_args = ['-Wall', '-Werror', '-O3', '-std=gnu99']
 
if '--time_wait' in sys.argv:
    compile_args.append('-DTIME_WAIT')
    sys.argv.remove('--time_wait')

if np.version.version < '1.7.0':
    compile_args.append('-DOLD_NUMPY')

module1 = Extension('wrapper', 
        extra_compile_args = compile_args,
        sources = ['wrapper.c', 'utils.c', 'fw.c', 'stopwatch.c'])
 
setup (name = 'wrapper', 
        ext_modules = [module1],
        include_dirs = [np.get_include()])
