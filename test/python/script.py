import Luna
print("Hello!")

# test the creation of the celllib
cells = Luna.CellLib()

for c in cells:
    print("Cell name:", c.name)
