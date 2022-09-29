
create_clock -period 10 -name virtual_clk
set_input_delay 1 -clock [get_clocks virtual_clk] [get_ports {a* b*}]
set_output_delay -clock virtual_clk -max 1.5 [get_ports y*]
