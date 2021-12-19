create_region("core", 10000, 10000, 60000, 40000)
create_rows("core", 0, 10000, 8)
print("Floorplan and rows created")

load_verilog("test/files/verilog/adder8.v")

set_toplevel_module("adder8")

-- place the pins
xpos = 0
xinc = 4000
x = 10000
for idx=0,7 do
    pinName = "a[" .. tostring(idx) .. "]"
    place_instance(pinName, "adder8", x, 50000)
    x = x + xinc
    pinName = "b[" .. tostring(idx) .. "]"
    place_instance(pinName, "adder8", x, 50000)
    x = x + xinc    
end

xinc = 8000
x = 10000
for idx=0,7 do
    pinName = "y[" .. tostring(idx) .. "]"
    place_instance(pinName, "adder8", x, 10000)
    x = x + xinc
end

-- place the instances in the top level module
place_module("adder8", "core")
