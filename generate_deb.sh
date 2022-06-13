#!/bin/sh

# SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
#
# SPDX-License-Identifier: GPL-3.0-only

cd build
cpack -G DEB
