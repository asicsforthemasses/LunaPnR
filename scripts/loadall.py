import Luna
import LunaExtra

Luna.clear()

for lef in LunaExtra.ProjectLefFiles():
    print("LEF: ", lef)
    Luna.loadLef(lef)

for lib in LunaExtra.ProjectLibFiles():
    print("LIB: ", lib)
    Luna.loadLib(lib)

for verilog in LunaExtra.ProjectVerilogFiles():
    print("Verilog: ", verilog)
    Luna.loadVerilog(verilog)
