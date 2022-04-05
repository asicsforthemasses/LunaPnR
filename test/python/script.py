import unittest
import Luna


class SimpleTestCases(unittest.TestCase):    
    
    # test the creation of the celllib
    def createCellLib(self):
        cells = Luna.CellLib()
        self.assertTrue(str(cells) == "Cells")
        print("Luna.CellLib prints as: ", cells)

    # test the creation of a cell
    def createCell(self):
        myCell = Luna.Cell()
        self.assertTrue(str(myCell) == "Cell")
        print("Luna.Cell prints as: ", myCell)

    # test the creation of an instance
    def createInstance(self):
        myInstance = Luna.Instance()
        self.assertTrue(str(myInstance) == "Instance")
        print("Luna.Instance prints as: ", myInstance)

    # 
    # test creation of pininfo
    def createPinInfo(self):
        pininfo = Luna.PinInfo()
        self.assertTrue(str(pininfo) == "PinInfo")
        print("Luna.PinInfo prints as: ", pininfo)        

class CellLibIterationTest(unittest.TestCase):

    # test celllib and cell pin iteration
    def testCellAndPinIteration(self):
        Luna.clear()
        cells = Luna.CellLib()
        for c in cells:
            print("Cell:", c.name)
            print("\tArea     ", c.area, "um^2")
            print("\tPower    ", c.leakagePower, "W")
            print("\tSize     ", c.size, "nm")
            print("\tOffset   ", c.offset, "nm")
            print("\tClass    ", c.cellClass)
            print("\tSubclass ", c.cellSubClass)
            print("\tSymmetry ", c.symmetry)

            for pin in c.pins:
                print("\t\t Pin: ", pin.name)
                print("\t\t\tIO type     : ", pin.ioType)
                print("\t\t\tCapacitance : ", pin.capacitance, " Farad")
                print("\t\t\tMax. cap    : ", pin.maxCapacitance, " Farad")
                print("\t\t\tClock       : ", pin.clock)
                print("\t\t\tOffset      : ", pin.offset)        
                print("\t\t\tFunction    : ", pin.function)
                print("\t\t\t3-Function  : ", pin.tristateFunction)

class TestImporters(unittest.TestCase):
    
    def testLEF(self):
        Luna.clear()
        Luna.loadLef("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef")

    def testLib(self):    
        Luna.clear()
        Luna.loadLib("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib")
    
    def testVerilog(self):
        Luna.clear()
        Luna.loadLef("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef")
        Luna.loadLib("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib")
        Luna.loadVerilog("test/files/verilog/adder8.v")


class CellLibLookupTests(unittest.TestCase):

    def testCellLookups(self):
        Luna.clear()
        Luna.loadLef("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef")
        Luna.loadLib("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib")
        Luna.loadVerilog("test/files/verilog/adder8.v")
        Luna.setTopModule("adder8")

        # get an instance by name
        instances = Luna.Instances()
        myInstance = instances.getInstance("_21_")
        self.assertTrue(myInstance.name == "_21_")
        self.assertTrue(myInstance.archetype == "BUFX2")
        print("Lookup of instance _21_ returns: ", myInstance.name, " ", myInstance.archetype)

        # check we can access a pin of an instance
        self.assertTrue(myInstance.getPinCount() == 4)
        myInputPin = myInstance.getPin(0)
        myGndPin = myInstance.getPin(1)
        myOutputPin = myInstance.getPin(2)
        myVccPin = myInstance.getPin(3)
        self.assertTrue(myInputPin.name == "A")
        self.assertTrue(myInputPin.pinInfo.ioType == "INPUT")
        self.assertTrue(myOutputPin.name == "Y")
        self.assertTrue(myOutputPin.pinInfo.ioType == "OUTPUT")

        # test that the input and output pins have a net associated with them
        self.assertTrue(myInputPin.getNetKey() >= 0)
        self.assertTrue(myOutputPin.getNetKey() >= 0)

        # test that the pwr/gnd pins are unconnected
        self.assertTrue(myGndPin.getNetKey() < 0)
        self.assertTrue(myVccPin.getNetKey() < 0)

        # check a non-existing instance lookup by name throws an exceptions
        with self.assertRaises(ValueError):
            instances.getInstance("ClaireDanes")

        # check a non-existing instance lookup by key throws an exceptions
        with self.assertRaises(ValueError):
            instances.getInstance(54321)

        # get a cell by name
        myCell = Luna.CellLib().getCell("BUFX2")
        self.assertTrue(myCell.name == "BUFX2")
        print("Lookup of cell BUFX2 returns: ", myCell.name)

        # check a non-existing cell lookup by name throws an exceptions
        with self.assertRaises(ValueError):
            Luna.CellLib().getCell("SalmaHayek")

        # check a non-existing cell lookup by key throws an exceptions
        with self.assertRaises(ValueError):
            Luna.CellLib().getCell(54321)


class TestNetlistAccess(unittest.TestCase):

    def test(self):
        Luna.clear()
        Luna.loadLef("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef")
        Luna.loadLib("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib")
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



if (__name__ == "__main__"):
    unittest.main()
