#!/bin/bash

cd /home/hallliu
git clone git@github.com:hallliu/w2014.git
cd ./w2014/cs230/hw1/

./setup.py build_ext -i
./perf.py

rm -r build
rm wrapper.so

./setup.py build_ext -i --time_wait
./perf.py time_wait

tar -zcvf results.tar.gz results
