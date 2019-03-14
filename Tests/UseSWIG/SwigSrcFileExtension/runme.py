#!/usr/bin/env python

from __future__ import print_function
import random

import my_add
import my_sub


# These can be changed, but make sure not to overflow `int`
a = random.randint(1, 1024)
b = random.randint(1, 1024)

if my_add.add(a, b) == a + b:
    print ("Test 1 Passed for SWIG custom source file extension")
else:
    print ("Test 1 FAILED for SWIG custom source file extension")
    exit(1)

if my_sub.sub(a, b) == a - b:
    print ("Test 2 Passed for SWIG custom source file extension")
else:
    print ("Test 2 FAILED for SWIG custom source file extension")
    exit(1)
