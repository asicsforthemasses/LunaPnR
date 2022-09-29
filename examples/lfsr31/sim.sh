#!/bin/sh

ghdl -a lfsr31.vhd
ghdl -a lfsr31_tb.vhd
ghdl -e lfsr31_tb
ghdl -r lfsr31_tb --wave=lfsr31.ghw