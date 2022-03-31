import Luna
print("Hello!")

# test the creation of the celllib
cells = Luna.CellLib()

for c in cells:
    print("Cell:", c.name)
    print("\tArea     ", c.area, "um^2")
    print("\tPower    ", c.leakagePower, "W")
    print("\tSize     ", c.size, "nm")
    print("\tOffset   ", c.offset, "nm")
    print("\tClass    ", c.cellClass)
    print("\tSubclass ", c.cellSubClass)

help(Luna.Cell())
