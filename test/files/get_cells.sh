#!/bin/sh

## SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
##
## SPDX-License-Identifier: GPL-3.0-only

wget -nc https://vlsiarch.ecen.okstate.edu/flows/MOSIS_SCMOS/iit_stdcells_v2.3beta/iitcells_lib_2.3.tar.gz
tar -xzf iitcells_lib_2.3.tar.gz
# rm iitcells_lib_2.3.tar.gz
./clean_cells.sh

mkdir -p sky130
cd sky130
wget -nc https://github.com/asicsforthemasses/sky130_testing/raw/main/sky130_fd_sc_hd_liberty.tar.xz -O sky130_fd_sc_hd.tar.xz
wget -nc https://github.com/asicsforthemasses/sky130_testing/raw/main/sky130_fd_sc_hd.tlef -O sky130_fd_sc_hd.tlef
wget -nc https://github.com/asicsforthemasses/sky130_testing/raw/main/sky130_fd_sc_hd.lef -O sky130_fd_sc_hd.lef
tar -xf sky130_fd_sc_hd.tar.xz
xz sky130_fd_sc_hd.xz
cd ..

mkdir -p nangate
wget -nc https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Low_Power/Front_End/Liberty/LowPowerOpenCellLibrary_functional.lib -O nangate/lpocl_functional.lib
wget -nc https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Front_End/Liberty/NangateOpenCellLibrary_functional.lib -O nangate/ocl_functional.lib
wget -nc https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Back_End/lef/NangateOpenCellLibrary.lef -O nangate/ocl.lef
wget -nc https://github.com/JulianKemmerer/Drexel-ECEC575/raw/master/Encounter/NangateOpenCellLibrary/Back_End/gds/NangateOpenCellLibrary.gds -O nangate/ocl.gds
wget -nc https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Back_End/lef/NangateOpenCellLibrary.tech.lef -O nangate/ocl.tech.lef

mkdir -p ihp130
cd ihp130
wget -nc https://github.com/IHP-GmbH/IHP-Open-PDK/raw/main/ihp-sg13g2/libs.ref/sg13g2_stdcell/lib/sg13g2_stdcell_fast_1p32V_m40C.lib
wget -nc https://github.com/IHP-GmbH/IHP-Open-PDK/raw/main/ihp-sg13g2/libs.ref/sg13g2_stdcell/lib/sg13g2_stdcell_fast_1p65V_m40C.lib
wget -nc https://github.com/IHP-GmbH/IHP-Open-PDK/raw/main/ihp-sg13g2/libs.ref/sg13g2_stdcell/lib/sg13g2_stdcell_slow_1p08V_125C.lib
wget -nc https://github.com/IHP-GmbH/IHP-Open-PDK/raw/main/ihp-sg13g2/libs.ref/sg13g2_stdcell/lib/sg13g2_stdcell_slow_1p35V_125C.lib
wget -nc https://github.com/IHP-GmbH/IHP-Open-PDK/raw/main/ihp-sg13g2/libs.ref/sg13g2_stdcell/lib/sg13g2_stdcell_typ_1p20V_25C.lib
wget -nc https://github.com/IHP-GmbH/IHP-Open-PDK/raw/main/ihp-sg13g2/libs.ref/sg13g2_stdcell/lib/sg13g2_stdcell_typ_1p50V_25C.lib
wget -nc https://github.com/IHP-GmbH/IHP-Open-PDK/raw/main/ihp-sg13g2/libs.ref/sg13g2_stdcell/lef/sg13g2_stdcell.lef
wget -nc https://github.com/IHP-GmbH/IHP-Open-PDK/raw/main/ihp-sg13g2/libs.ref/sg13g2_stdcell/lef/sg13g2_tech.lef
(exit 0)
