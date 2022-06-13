#!/usr/bin/python3

# SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
#
# SPDX-License-Identifier: GPL-3.0-only

# Draw an ASCII mandelbrot set

import cmath
import numpy as np

def mandelIterations(c : complex) -> int:
    maxIter = 64
    z = c
    for iter in range(0,maxIter):
        magnitudeSquared = z*z.conjugate()
        if (magnitudeSquared.real >= 4.0):
            return iter
        z = z*z + c
    return maxIter

def mandel():
    for y in np.linspace(-1,1,40):
        for x in np.linspace(-2,1,80):
            c = complex(x,y)
            iters = mandelIterations(c)
            if (iters > 10):
                print("*", end = '')
            else:
                print(" ", end = '')
        print()


if __name__ == '__main__':
    # Script2.py executed as script
    # do something
    mandel()

