clear()
load_lef("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef")
load_lef("test/files/iit_stdcells_extra/fake_ties018.lef")
load_lib("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib")
load_lib("test/files/iit_stdcells_extra/fake_ties018.lib")
load_layers("scripts/osu180_layers.json")
print("OSU180 technology loaded")

