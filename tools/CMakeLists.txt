# SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
#
# SPDX-License-Identifier: GPL-3.0-only

project(doctool)

add_executable(doctool ${PROJECT_SOURCE_DIR}/doctool.cpp)
add_executable(fixme ${PROJECT_SOURCE_DIR}/fixme.cpp)
add_executable(spdx ${PROJECT_SOURCE_DIR}/spdx.cpp)
