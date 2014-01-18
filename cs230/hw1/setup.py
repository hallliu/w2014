#!/usr/bin/python2
from distutils.core import setup, Extension
import numpy as np
 
module1 = Extension('wrapper', 
        extra_compile_args = ['-Wall', '-Werror', '-g', '-std=gnu99'],
        sources = ['wrapper.c', 'utils.c', 'fw.c', 'stopwatch.c'])
 
setup (name = 'wrapper', 
        ext_modules = [module1],
        include_dirs = [np.get_include()])
