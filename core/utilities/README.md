# LunaPnR Core Utilities

## Netlistgen

A simple netlist generator meant for testing.

### Netlist format
* number of instances, EOL.
* number of nets, EOL.

For each net:
* number of connections.
* instance ID.
* EOL.

Instance IDs are not necessarily contiguous, i.e. gaps may appear after netlist transformations by LunaPnR.
