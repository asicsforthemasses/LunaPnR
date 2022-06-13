# SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
#
# SPDX-License-Identifier: GPL-3.0-only

createRegion("core", 10000, 10000, 60000, 40000)
createRows("core", 0, 10000, 8)
print("Floorplan and rows created")

loadVerilog("test/files/verilog/adder8.v")

setTopModule("adder8")

## place the pins
xpos = 0
xinc = 4000
x = 10000
for idx in range(0,8):
    pinName = "a[" + str(idx) + "]"
    placeInstance(pinName, "adder8", x, 50000)
    x = x + xinc
    pinName = "b[" + str(idx) + "]"
    placeInstance(pinName, "adder8", x, 50000)
    x = x + xinc

xinc = 8000
x = 10000
for idx in range(0,8):
    pinName = "y[" + str(idx) + "]"
    placeInstance(pinName, "adder8", x, 10000)
    x = x + xinc

## place the instances in the top level module
placeModule("adder8", "core")
