
create_region("core", 10000, 10000, 600000, 600000)
create_rows("core", 0, 10000, 60)
print("Floorplan and rows created")

load_verilog("test/files/verilog/femtorv32_quark.v")

set_toplevel_module("FemtoRV32")

xTopPos = 10000
xTopInc = 5000

xBottomPos = 10000
xBottomInc = 5000

yTop    = 610000
yBottom = 10000

function placePinAtTop(pinName)
    place_instance(pinName, "FemtoRV32", xTopPos, yTop)
    xTopPos = xTopPos + xTopInc
end

function placePinAtBottom(pinName)
    place_instance(pinName, "FemtoRV32", xBottomPos, yBottom)
    xBottomPos = xBottomPos + xBottomInc
end

placePinAtTop("clk")
placePinAtTop("reset")
for num=0,31 do
    placePinAtTop("mem_addr[" .. tostring(num) .. "]")
end

for num=0,31 do
    placePinAtTop("mem_wdata[" .. tostring(num) .. "]")
end

placePinAtBottom("mem_wmask[0]")
placePinAtBottom("mem_wmask[1]")
placePinAtBottom("mem_wmask[2]")
placePinAtBottom("mem_wmask[3]")
placePinAtBottom("mem_wbusy")

for num=0,31 do
    placePinAtBottom("mem_rdata[" .. tostring(num) .. "]")
end

placePinAtBottom("mem_rstrb")
placePinAtBottom("mem_rbusy")

-- place the instances in the top level module
place_module("FemtoRV32", "core")

write_density_bitmap("FemtoRV32", "core", "bitmap.pgm")
