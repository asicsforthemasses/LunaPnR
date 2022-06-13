# SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
#
# SPDX-License-Identifier: GPL-3.0-only

createRegion("core", 10000, 10000, 80000, 80000)
createRows("core", 0, 10000, 8)
print("Floorplan and rows created")

loadVerilog("test/files/verilog/multiplier.v")

setTopModule("multiplier")

## place the pins
xpos = 0
xinc = 10000
x = 10000
for idx in range(0,4): 
    pinName = "a_in[" + str(idx) + "]"
    placeInstance(pinName, "multiplier", x, 90000)
    x = x + xinc
    pinName = "b_in[" + str(idx) + "]"
    placeInstance(pinName, "multiplier", x, 90000)
    x = x + xinc    

xinc = 10000
x = 10000
for idx in range(0,8):
    pinName = "data_out[" + str(idx) + "]"
    placeInstance(pinName, "multiplier", x, 10000)
    x = x + xinc

## place the instances in the top level module
placeModule("multiplier", "core")
