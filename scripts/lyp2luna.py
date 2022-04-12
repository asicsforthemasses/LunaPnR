#!/usr/bin/python3

import xml.etree.ElementTree as ET

tree = ET.parse('sky130.lyp')
root = tree.getroot()

patdict = {}
layerdict = {}

cpatterns = root.findall("./custom-dither-pattern")
for pat in cpatterns:

    patname = "C" + pat.findall('./order')[0].text

    lines = pat.findall('./pattern/line')

    patstr = ""
    ysize = 0
    for line in lines:
        xsize = len(line.text)
        ysize += 1
        patstr += line.text

    patdict[patname] = (patstr, xsize, ysize)

layers = root.findall("./properties")
for layer in layers:
    name       = layer.findall('./name')[0].text
    framecolor = layer.findall('./frame-color')[0].text
    fillcolor  = layer.findall('./fill-color')[0].text
    linestyle  = layer.findall('./line-style')[0].text
    hatchpatid = layer.findall('./dither-pattern')[0].text

    name = name.split()[0]

    hatchpattern = ("",0,0)
    if hatchpatid[0] == 'C':
        if hatchpatid in patdict:
            hatchpattern = patdict[hatchpatid]

    layerdict[name] = (framecolor, fillcolor, linestyle, hatchpattern)


# export a couple of layers
layers = ("li1","met1","met2","met3","met4","met5","nwell","pwell","via","via2","via3","via4","mcon")


for layer in layers:

    template = """ 
        {
            "layer": "#NAME1",
            "types": [
                {
                    "color": "#FILL1",
                    "height": #YSIZE1,
                    "pixmap": "#PATSTR1",
                    "width": #YSIZE1
                },
                {
                    "color": "#FILL2",
                    "height": #YSIZE2,
                    "pixmap": "#PATSTR2",
                    "width": #XSIZE2
                }
            ]
        }, 
"""

    part1 = layerdict[layer + ".drawing"]
    if (layer + ".blockage" in layerdict):
        part2 = layerdict[layer + ".blockage"]
    else:
        part2 = part1

    template = template.replace("#NAME1", layer)
    template = template.replace("#FILL1", part1[1])
    template = template.replace("#PATSTR1", part1[3][0])
    template = template.replace("#XSIZE1", str(part1[3][1]))
    template = template.replace("#YSIZE1", str(part1[3][2]))

    template = template.replace("#FILL2", part2[1])
    template = template.replace("#PATSTR2", part2[3][0])
    template = template.replace("#XSIZE2", str(part2[3][1]))
    template = template.replace("#YSIZE2", str(part2[3][2]))

    print(template)
