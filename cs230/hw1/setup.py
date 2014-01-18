#!/usr/bin/python2
from distutils.core import setup, Extension
import sys
import numpy as np
 
if '--time_wait' in sys.argv:
    module1 = Extension('wrapper', 
            extra_compile_args = ['-Wall', '-Werror', '-O3', '-std=gnu99', '-DTIME_WAIT'],
            sources = ['wrapper.c', 'utils.c', 'fw.c', 'stopwatch.c'])
    sys.argv.remove('--time_wait')
else:
    module1 = Extension('wrapper', 
            extra_compile_args = ['-Wall', '-Werror', '-O3', '-std=gnu99'],
            sources = ['wrapper.c', 'utils.c', 'fw.c', 'stopwatch.c'])
 
setup (name = 'wrapper', 
        ext_modules = [module1],
        include_dirs = [np.get_include()])
