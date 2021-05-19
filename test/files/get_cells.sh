#!/bin/sh

wget https://vlsiarch.ecen.okstate.edu/flows/MOSIS_SCMOS/iit_stdcells_v2.3beta/iitcells_lib_2.3.tar.gz
tar -xzf iitcells_lib_2.3.tar.gz
rm iitcells_lib_2.3.tar.gz
./clean_cells.sh
