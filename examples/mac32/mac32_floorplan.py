# SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
#
# SPDX-License-Identifier: GPL-3.0-only

setLogLevel("VERBOSE")

cell_height = 10000
cell_width  = 800

rows = 100
ys = cell_height * rows
xs = ys

xofs = 50000
yofs = 50000
pin_distance = 40000

## create floorplan
createRegion("core", "core", xofs, yofs, xs, ys)

setTopModule("mac32")

## place the pins
xpos = 0
xinc = 16000
x = xofs+10000

for idx in range(0,32): 
    pinName = "a_in[" + str(idx) + "]"
    placeInstance(pinName, "mac32", x, yofs+ys+pin_distance)
    x = x + xinc
    pinName = "b_in[" + str(idx) + "]"
    placeInstance(pinName, "mac32", x, yofs+ys+pin_distance)
    x = x + xinc    

xinc = 16000
x = xofs+10000

placeInstance("clk", "mac32", x, yofs-pin_distance)
x = x + xinc
placeInstance("rst_an", "mac32", x, yofs-pin_distance)
x = x + xinc

for idx in range(0,64):
    pinName = "data_out[" + str(idx) + "]"
    placeInstance(pinName, "mac32", x, yofs-pin_distance)
    x = x + xinc
