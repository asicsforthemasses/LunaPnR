
create_clock -period 10 clk
set_input_delay 1 -clock [get_clocks clk] [get_ports {rst_an}]
set_output_delay -clock clk -max 1.5 [get_ports {dout}]
