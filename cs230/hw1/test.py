#!/usr/bin/python2
import wrapper
import numpy as np

def test_load_csv():
    filename = '/tmp/TEST_LOAD_CSV_hallliu.csv'
    a = np.random.randint(10000000, size=(40,40))
    b = np.empty((40,40), dtype='int32')
    np.savetxt(filename, a, delimiter=',', fmt='%d')
    wrapper.from_csv(filename, b, 40)
    assert np.all(a == b)

def test_dump_csv():
    filename = '/tmp/TEST_DUMP_CSV_hallliu.csv'
    a = np.random.randint(10000000, size=(40,40)).astype('int32')
    wrapper.to_csv(filename, a, 40)
    b = np.loadtxt(filename, delimiter=',', dtype='int32')
    assert np.all(a == b)

test_load_csv()
test_dump_csv()
