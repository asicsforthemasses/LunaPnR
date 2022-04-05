import Luna
print("Hello!")

# test the creation of the celllib
cells = Luna.CellLib()
print("Luna.CellLib prints as: ", cells)

# test the creation of a cell
myCell = Luna.Cell()
print("Luna.Cell prints as: ", myCell)

# test the creation of an instance
myInstance = Luna.Instance()
print("Luna.Instance prints as: ", myInstance)

# test celllib and cell pin iteration
for c in cells:
    print("Cell:", c.name)
    print("\tArea     ", c.area, "um^2")
    print("\tPower    ", c.leakagePower, "W")
    print("\tSize     ", c.size, "nm")
    print("\tOffset   ", c.offset, "nm")
    print("\tClass    ", c.cellClass)
    print("\tSubclass ", c.cellSubClass)
    print("\tSymmetry ", c.symmetry)

    #pins = c.pins
    for pin in c.pins:
        print("\t\t Pin: ", pin.name)
        print("\t\t\tIO type     : ", pin.ioType)
        print("\t\t\tCapacitance : ", pin.capacitance, " Farad")
        print("\t\t\tMax. cap    : ", pin.maxCapacitance, " Farad")
        print("\t\t\tClock       : ", pin.clock)
        print("\t\t\tOffset      : ", pin.offset)        
        print("\t\t\tFunction    : ", pin.function)
        print("\t\t\t3-Function  : ", pin.tristateFunction)
        
# test creation of pininfo
pininfo = Luna.PinInfo()
print("Luna.PinInfo prints as: ", pininfo)

# load LEF and LIB
Luna.loadLib("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib")
Luna.loadLef("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef")

# load verilog
Luna.loadVerilog("test/files/verilog/adder8.v")

Luna.setTopModule("adder8")

x = 1000
for ins in Luna.Instances():
    if not(ins.archetype == "__PIN"):
        print(ins.name, "->", ins.archetype, " size:", ins.size," pos", ins.position, " placement:", ins.placementInfo, " orientation:", ins.orientation)
    else:
        # test setting the positiob
        ins.position = (x,1000)
        ins.placementInfo = "PLACED"
        x = x + 1000
        print(ins.name, "->", ins.archetype, " pos", ins.position)

    for key in range(0, ins.getPinCount()):
        pin = ins.getPin(key)
        print("\tpin:", pin.name, " connected net key:", pin.getNetKey(), " pin type:", pin.pinInfo.ioType)

# get an instance by name
instances = Luna.Instances()
myInstance = instances.getInstance("_21_")
print("Lookup of instance _21_ returns: ", myInstance.name, " ", myInstance.archetype)

# get a cell by name
myCell = Luna.CellLib().getCell("BUFX2")
print("Lookup of cell BUFX2 returns: ", myCell.name)
