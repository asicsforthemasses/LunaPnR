# SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
#
# SPDX-License-Identifier: GPL-3.0-only

clear()
loadLef("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef")
loadLef("test/files/iit_stdcells_extra/fake_ties018.lef")
loadLib("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib")
loadLib("test/files/iit_stdcells_extra/fake_ties018.lib")
loadLayers("scripts/osu180_layers.json")
print("OSU180 technology loaded")
