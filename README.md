# GSModeSelector

## Description

Play your PS1 games on modern TV sets! 
- automatic disc region control
- automatic GSM configuration for correct display of PS1 games
- automatic disc start-up function
- Compatible with HDMI adapter/component cable. 

## Compiling and usage

1. To build this (only for Windows), you'll need the [Pre-built homebrew PlayStation 2 MinGW toolchain](https://github.com/ps2dev/ps2toolchain/releases/tag/2018-10-19).
2. Extract the 7z file somewhere (I extracted it to my HDD, which is D:/).
3. Go to MinGW/msys/1.0 and open msys.bat.
4. Now you can cd to the git cloned/downloaded folder of this fork. (In my case, I'd type cd D:/Users/Sestain/Desktop/shared/gsm-master)
5. Once you are in the folder where the source is, then you can do make and it should compile and create a GSM.ELF file.
6. Now you can run the .ELF file on PCSX2 with Run ELF or on real hardware with wLaunchElf (On real hardware you have to use YPbPr output setting so it displays correctly and doesn't crash).

## Credits
Thanks to [Sestain](https://github.com/sestain) for their fork of GSM.
Thanks to [taka-tuos]() for linking Pre-built toolchain in their fork of GSM.
Thanks to [taka-tuos](https://github.com/taka-tuos) for linking Pre-built toolchain in their fork of GSM.
Thanks to [doctorxyz](https://github.com/doctorxyz), dlanor, [SP193](https://www.psx-place.com/members/sp193.19/), reprep for GSM.
