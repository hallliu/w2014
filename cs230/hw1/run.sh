#!/bin/bash

rm -r build
rm wrapper.so

./setup.py build_ext -i
./perf.py

rm -r build
rm wrapper.so

./setup.py build_ext -i --time_wait
./perf.py time_wait
