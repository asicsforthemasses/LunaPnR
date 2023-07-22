#!/bin/sh
## SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
##
## SPDX-License-Identifier: GPL-3.0-only

sudo apt install -y python3-pip python3-setuptools patchelf desktop-file-utils libgtk-pixbuf2.0-dev fakeroot strace
sudo wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -O /usr/local/bin/appimagetool
sudo chmod +x /usr/local/bin/appimagetool
sudo pip3 install appimage-builder

mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/share/icons/hicolor/128x128/apps/
mkdir -p AppDir/usr/share/applications

cp build/gui/lunapnr AppDir/usr/bin
cp resources/icons/lunapnr.png AppDir/usr/share/icons/hicolor/128x128/apps/
cp resources/lunapnr.desktop AppDir/usr/share/applications/

appimage-builder --recipe AppImageBuilder.yml

rm -rf AppDir
rm -rf appimage-builder-cache
