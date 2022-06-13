#!/bin/sh

# SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
#
# SPDX-License-Identifier: GPL-3.0-only

rm -rf build
mkdir -p build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DUseCPPCHECK=ON ..
