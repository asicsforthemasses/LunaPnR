#!/bin/sh

wget https://vlsiarch.ecen.okstate.edu/flows/MOSIS_SCMOS/iit_stdcells_v2.3beta/iitcells_lib_2.3.tar.gz
tar -xzf iitcells_lib_2.3.tar.gz
rm iitcells_lib_2.3.tar.gz
./clean_cells.sh

mkdir nangate
wget https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Low_Power/Front_End/Liberty/LowPowerOpenCellLibrary_functional.lib -O nangate/lpocl_functional.lib
wget https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Front_End/Liberty/NangateOpenCellLibrary_functional.lib -O nangate/ocl_functional.lib
wget https://raw.githubusercontent.com/JulianKemmerer/Drexel-ECEC575/master/Encounter/NangateOpenCellLibrary/Back_End/lef/NangateOpenCellLibrary.lef -O nangate/ocl.lef
wget https://github.com/JulianKemmerer/Drexel-ECEC575/raw/master/Encounter/NangateOpenCellLibrary/Back_End/gds/NangateOpenCellLibrary.gds -O nangate/ocl.gds
