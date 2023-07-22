#!/bin/sh
#
# SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
#
# SPDX-License-Identifier: GPL-3.0-only

#EXPORT PDK_PATH="/storage/programming/asic/luna_pnr/test.files/"
yosys -s mac32.tcl
