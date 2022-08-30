# SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
#
# SPDX-License-Identifier: GPL-3.0-only

## create floorplan
createRegion("core", 10000, 10000, 650000, 650000)
createRows("core", 0, 10000, 64)

setTopModule("mac32")

## place the pins
xpos = 0
xinc = 10000
x = 10000

for idx in range(0,32): 
    pinName = "a_in[" + str(idx) + "]"
    placeInstance(pinName, "mac32", x, 650000)
    x = x + xinc
    pinName = "b_in[" + str(idx) + "]"
    placeInstance(pinName, "mac32", x, 650000)
    x = x + xinc    

xinc = 10000
x = 10000

placeInstance("clk", "mac32", x, 100000)
x = x + xinc
placeInstance("rst_an", "mac32", x, 10000)
x = x + xinc

for idx in range(0,64):
    pinName = "data_out[" + str(idx) + "]"
    placeInstance(pinName, "mac32", x, 10000)
    x = x + xinc
