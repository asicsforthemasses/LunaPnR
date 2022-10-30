create_clock -name clk -period 12 {clk}
set_input_delay  -clock {clk} -max 1 [all_inputs]
set_output_delay  -clock {clk} -max 1 [all_outputs]
