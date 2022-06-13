REM SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
REM
REM SPDX-License-Identifier: GPL-3.0-only

set BOOST_ROOT=C:\Boost
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
cd ..