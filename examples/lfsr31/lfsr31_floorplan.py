# SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
#
# SPDX-License-Identifier: GPL-3.0-only

## create floorplan
createRegion("core", "core", 10000, 10000, 170000, 50000)

setTopModule("LFSR31")

## place the pins
placeInstance("clk",    "LFSR31",500, 15000)
placeInstance("rst_an", "LFSR31",500, 25000)
placeInstance("dout",   "LFSR31",200000, 20000)
