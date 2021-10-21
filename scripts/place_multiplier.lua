create_region("core", 10000, 10000, 80000, 80000)
create_rows("core", 0, 10000, 8)
print("Floorplan and rows created")

load_verilog("test/files/verilog/multiplier.v")

set_toplevel_module("multiplier")

-- place the pins
xpos = 0
xinc = 10000
x = 10000
for idx=0,3 do
    pinName = "a_in[" .. tostring(idx) .. "]"
    place_instance(pinName, "multiplier", x, 90000)
    x = x + xinc
    pinName = "b_in[" .. tostring(idx) .. "]"
    place_instance(pinName, "multiplier", x, 90000)
    x = x + xinc    
end

xinc = 10000
x = 10000
for idx=0,7 do
    pinName = "data_out[" .. tostring(idx) .. "]"
    place_instance(pinName, "multiplier", x, 10000)
    x = x + xinc
end

-- place the instances in the top level module
place_module("multiplier", "core")
