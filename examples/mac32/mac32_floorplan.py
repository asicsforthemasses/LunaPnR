# SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
#
# SPDX-License-Identifier: GPL-3.0-only

setLogLevel("VERBOSE")

## create floorplan
createRegion("core", "core", 10000, 10000, 650400, 650000)

setTopModule("mac32")

## place the pins
xpos = 0
xinc = 10000
x = 10000

for idx in range(0,32): 
    pinName = "a_in[" + str(idx) + "]"
    placeInstance(pinName, "mac32", x, 660000)
    x = x + xinc
    pinName = "b_in[" + str(idx) + "]"
    placeInstance(pinName, "mac32", x, 660000)
    x = x + xinc    

xinc = 10000
x = 10000

placeInstance("clk", "mac32", x, 100000)
x = x + xinc
placeInstance("rst_an", "mac32", x, 0)
x = x + xinc

for idx in range(0,64):
    pinName = "data_out[" + str(idx) + "]"
    placeInstance(pinName, "mac32", x, 0)
    x = x + xinc
