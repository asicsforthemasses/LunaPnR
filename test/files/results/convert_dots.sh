#!/bin/sh

# SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
#
# SPDX-License-Identifier: GPL-3.0-only

dot -Tpng qlanetlist.dot -o qlanetlist.png
dot -Tpng adder8.dot -o adder8.png
dot -Tpng multiplier_after.dot -o multiplier_after.png