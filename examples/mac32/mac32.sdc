create_clock -name clk -period 10
set_input_delay -clock clk 0 [get_ports a_in*]
set_input_delay -clock clk 0 [get_ports b_in*]
set_output_delay 5 -clock clk [get_ports data_out*]
