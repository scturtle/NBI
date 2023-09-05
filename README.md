![Main Page](https://i.imgur.com/QOi0Yvv.jpg)

## Features
- Installs NSP/NSZ/XCI/XCZ files and split NSP/XCI files from your SD card.
- Installs NSP/NSZ/XCI/XCZ files over LAN or USB from tools such as [NS-USBloader](https://github.com/developersu/ns-usbloader).
- Installs Split NSP/XCI/NSZ/XCZ over Lan or USB using [NS-USBloader(Mod)](https://mega.nz/file/I4p2gCCK#32GwAGtIcL3FVH-V-8Goae_hpnK8FQ0eS2PwLDOW6X4).
- Installs NSP/NSZ/XCI/XCZ files over the internet by web server/Google Drive/pastebin etc.
- Installs NSP/NSZ/XCI/XCZ files from a USB hard drive (NTFS/Fat32/ExFat/EXT3/EXT4).
- Verifies NCAs by header signature before they're installed.
- Works on SX OS and Atmosphere.
- Able to theme, change install sounds.

## Thanks to
- Blawar, Hunterweb, DarkMatterCore, XorTroll

## Modified Code
This code was prominently modified by MrDude on 05/09/2023 to be able to build with new plutonium and up to date libnx.

## Building All componenets of TinWoo at once
cd into the tinwoo folder then "make".

## Build TinWoo components individually
First, build and install libusbhsfs - "make fs-libs" then "make BUILD_TYPE=GPL install".\
Second, built and install Plutonium - "make plutonium install".\
Third, Make Tinwoo - "make".

## Cleanup TinWoo once built
First, "make libusbclean".\
Second, "make cleanplutonium".\
Third, "make clean".

## Note
This is a work in progress and lets you build with new libnx, plutonium packages. Some stuff still needs fixed to work with the new plutonium and libnx changes.

## Stuff still to fix
I'll let you know when I get around to it.

## Build Issues
Make sure you are using the latest Libnx build from 5/9/23 or later.

git clone --recursive https://github.com/switchbrew/libnx.git \
cd libnx \
git checkout 4fcdb6eb \
make install

## Check libnx version
pacman -Q --info libnx
