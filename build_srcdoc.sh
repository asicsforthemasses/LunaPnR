#!/bin/sh
## SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
##
## SPDX-License-Identifier: GPL-3.0-only

cd doc
doxygen Doxyfile
ln -s srcdoc/html/index.html index.html
