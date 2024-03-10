# SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
#
# SPDX-License-Identifier: GPL-3.0-only

#setLogLevel("VERBOSE")

cell_height = 10000
cell_width  = 800

rows = 70
ys = cell_height * rows
xs = ys

xofs = 50000
yofs = 50000
pin_distance = 40000

## create floorplan
createRegion("core", "core", xofs, yofs, xs, ys)

setTopModule("FemtoRV32")

# clk,  \mem_addr[1] , \mem_addr[2] , \mem_addr[3] , \mem_addr[4] , \mem_addr[5] , \mem_addr[6] , \mem_addr[7] , \mem_addr[8] , \mem_addr[9] , \mem_addr[10] , \mem_addr[11] , \mem_addr[12] , \mem_addr[13] , \mem_addr[14] , \mem_addr[15] , \mem_addr[16] , \mem_addr[17] , \mem_addr[18] , \mem_addr[19] , \mem_addr[20] , \mem_addr[21] , \mem_addr[22] , \mem_addr[23] , \mem_addr[24] , \mem_addr[25] , \mem_addr[26] , \mem_addr[27] , \mem_addr[28] , \mem_addr[29] , \mem_addr[30] , \mem_addr[31] ,
# \mem_wdata[0] , \mem_wdata[1] , \mem_wdata[2] , \mem_wdata[3] , \mem_wdata[4] , \mem_wdata[5] , \mem_wdata[6] , \mem_wdata[7] , \mem_wdata[8] , \mem_wdata[9] , \mem_wdata[10] , \mem_wdata[11] , \mem_wdata[12] , \mem_wdata[13] , \mem_wdata[14] , \mem_wdata[15] , \mem_wdata[16] , \mem_wdata[17] , \mem_wdata[18] , \mem_wdata[19] , \mem_wdata[20] , \mem_wdata[21] , \mem_wdata[22] , \mem_wdata[23] , \mem_wdata[24] , \mem_wdata[25] , \mem_wdata[26] , \mem_wdata[27] , \mem_wdata[28] , \mem_wdata[29] , \mem_wdata[30] , \mem_wdata[31] ,
# \mem_wmask[0] , \mem_wmask[1] , \mem_wmask[2] , \mem_wmask[3] ,
# \mem_rdata[0] , \mem_rdata[1] , \mem_rdata[2] , \mem_rdata[3] , \mem_rdata[4] , \mem_rdata[5] , \mem_rdata[6] , \mem_rdata[7] , \mem_rdata[8] , \mem_rdata[9] , \mem_rdata[10] , \mem_rdata[11] , \mem_rdata[12] , \mem_rdata[13] , \mem_rdata[14] , \mem_rdata[15] , \mem_rdata[16] , \mem_rdata[17] , \mem_rdata[18] , \mem_rdata[19] , \mem_rdata[20] , \mem_rdata[21] , \mem_rdata[22] , \mem_rdata[23] , \mem_rdata[24] , \mem_rdata[25] , \mem_rdata[26] , \mem_rdata[27] , \mem_rdata[28] , \mem_rdata[29] , \mem_rdata[30] , \mem_rdata[31] ,
# mem_rstrb, mem_rbusy, mem_wbusy, reset);

## place the pins
xpos = 0
xinc = 16000
yinc = 10000*2
x = xofs+10000

topNames = ["clk", "reset", "mem_rstrb", "mem_rbusy", "mem_wbusy"]

for idx in range(0,32):
    pname = "mem_addr[{}]".format(idx)
    topNames.append(pname)

for idx in range(0,4):
    pname = "mem_wmask[{}]".format(idx)
    topNames.append(pname)

rightNames = []

for idx in range(0,32):
    pname = "mem_wdata[{}]".format(idx)
    rightNames.append(pname)

bottomNames = []

for idx in range(0,32):
    pname = "mem_rdata[{}]".format(idx)
    bottomNames.append(pname)

for pinName in topNames:
    placeInstance(pinName, "FemtoRV32", x, yofs+ys+pin_distance)
    x = x + xinc

x = xofs+10000
for pinName in bottomNames:
    placeInstance(pinName, "FemtoRV32", x, yofs-pin_distance)
    x = x + xinc

y = yofs + ys
for pinName in rightNames:
    placeInstance(pinName, "FemtoRV32", xs + xofs + pin_distance, y)
    y = y - yinc
