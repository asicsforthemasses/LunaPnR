REM SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
REM
REM SPDX-License-Identifier: GPL-3.0-only

wget -nc https://vlsiarch.ecen.okstate.edu/flows/MOSIS_SCMOS/iit_stdcells_v2.3beta/iitcells_lib_2.3.tar.gz
tar -xzf iitcells_lib_2.3.tar.gz
rem ./clean_cells.sh

mkdir -p nangate
wget -nc https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Low_Power/Front_End/Liberty/LowPowerOpenCellLibrary_functional.lib -O nangate/lpocl_functional.lib
wget -nc https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Front_End/Liberty/NangateOpenCellLibrary_functional.lib -O nangate/ocl_functional.lib
wget -nc https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Back_End/lef/NangateOpenCellLibrary.lef -O nangate/ocl.lef
wget -nc https://github.com/JulianKemmerer/Drexel-ECEC575/raw/master/Encounter/NangateOpenCellLibrary/Back_End/gds/NangateOpenCellLibrary.gds -O nangate/ocl.gds
wget -nc https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Back_End/lef/NangateOpenCellLibrary.tech.lef -O nangate/ocl.tech.lef

Exit 0