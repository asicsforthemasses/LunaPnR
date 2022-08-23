// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#ifdef NO_SSIZE_T
#include <type_traits>
typedef std::make_signed<size_t>::type ssize_t;
#endif