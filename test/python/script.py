import Luna
print("Hello!")

Luna.loadLib("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib")
Luna.loadLef("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef")

# test the creation of the celllib
cells = Luna.CellLib()
print("Luna.CellLib prints as: ", cells)

# test the creation of the cell
myCell = Luna.Cell()
print("Luna.Cell prints as: ", myCell)

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

