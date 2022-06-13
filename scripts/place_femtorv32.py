# SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
#
# SPDX-License-Identifier: GPL-3.0-only

createRegion("core", 10000, 10000, 800000, 500000)
createRows("core", 0, 10000, 60)
print("Floorplan and rows created")

loadVerilog("test/files/verilog/femtorv32_quark.v")

setTopModule("FemtoRV32")

xTopPos = 10000
xTopInc = 10000

xBottomPos = 10000
xBottomInc = 15000

yTop    = 510000
yBottom = 10000

def placePinAtTop(pinName):
    global xTopPos
    placeInstance(pinName, "FemtoRV32", xTopPos, yTop)
    xTopPos = xTopPos + xTopInc

def placePinAtBottom(pinName):
    global xBottomPos
    placeInstance(pinName, "FemtoRV32", xBottomPos, yBottom)
    xBottomPos = xBottomPos + xBottomInc

placePinAtTop("clk")
placePinAtTop("reset")
for num in range(0,32):
    placePinAtTop("mem_addr[" + str(num) + "]")

for num in range(0,32):
    placePinAtTop("mem_wdata[" + str(num) + "]")

placePinAtBottom("mem_wmask[0]")
placePinAtBottom("mem_wmask[1]")
placePinAtBottom("mem_wmask[2]")
placePinAtBottom("mem_wmask[3]")
placePinAtBottom("mem_wbusy")

for num in range(0,32):
    placePinAtBottom("mem_rdata[" + str(num) + "]")

placePinAtBottom("mem_rstrb")
placePinAtBottom("mem_rbusy")

# place the instances in the top level module
placeModule("FemtoRV32", "core")

#write_density_bitmap("FemtoRV32", "core", "bitmap.pgm")
