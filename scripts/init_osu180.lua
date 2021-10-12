clear()
load_lef("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef")
load_lib("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib")
load_layers("scripts/osu180_layers.json")
print("OSU180 technology loaded")

create_region("core", 10000, 10000, 80000, 80000)
create_rows("core", 0, 10000, 8)
print("Floorplan and rows created")

load_verilog("test/files/verilog/adder8.v")