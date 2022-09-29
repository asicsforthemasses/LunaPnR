#!/bin/sh
yosys -m ghdl -p 'ghdl lfsr31.vhd -e LFSR31; write_verilog lfsr31_netlist.v'
