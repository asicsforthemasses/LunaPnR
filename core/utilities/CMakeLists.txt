# SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
#
# SPDX-License-Identifier: GPL-3.0-only

add_executable(netlistgen
    netlistgen.cpp)

target_link_libraries(netlistgen lunacore cxxopts)
