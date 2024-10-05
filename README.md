# PS1 HDMI/Component Fix for PS2

## Description

Play your PS1 games on modern TV sets! 
- automatic disc region control (PAL/NTSC)
- automatic GSM configuration for correct display of PS1 games
- automatic disc start-up function
- Perfect for playing PS1 games with HDMI/component cable. 




## Compiling and Usage

### Windows Compilation

1. To build this (only for Windows), you'll need the [Pre-built homebrew PlayStation 2 MinGW toolchain](https://github.com/ps2dev/ps2toolchain/releases/tag/2018-10-19).
2. Extract the 7z file in C:/.
3. Go to `MinGW/msys/1.0` and open `msys.bat`.
4. Now you can `cd` to the git cloned/downloaded folder of this fork.
5. Once you are in the folder where the source is, then you can run `make` and it should compile and create a `build` folder with `PS1_HDMI_FIX.ELF` and `uncompressed_PS1_HDMI_FIX` files inside.
6. You can run the `.ELF` file on PCSX2 with "Run ELF" or on real hardware with wLaunchElf. (On real hardware, you have to use the YPbPr output setting to ensure it displays correctly and doesn't crash).

### Docker Compilation

Alternatively, you can compile the project using a Docker container that provides the PS2 development environment:

1. Make sure Docker is installed and running on your system. 
   - You can download Docker from [here](https://www.docker.com/get-started).
   
2. Pull the PS2DEV Docker image by running the following command:
   ```bash
   docker pull ps2dev/ps2dev:v1.0
   ```

3. Mount the folder containing the source code and run the Docker container:
   ```bash
   docker run -it -w /PS2DEV -v /path/to/your/source/folder:/PS2DEV --name ps2dev_v1_0 ps2dev/ps2dev:v1.0 sh
   ```

   Replace `/path/to/your/source/folder` with the path to the folder where your source code is located. If you're on Windows, you can use the following command to make it user-agnostic:
   
   ```bash
   docker run -it -w /PS2DEV -v %USERPROFILE%/Desktop/PS2DEV:/PS2DEV --name ps2dev_v1_0 ps2dev/ps2dev:v1.0 sh
   ```

4. Once inside the container, install the necessary development tools and libraries:
   ```bash
   apk add build-base cmake git texinfo flex bison gettext gmp-dev mpfr-dev mpc1-dev zlib-dev nano
   ```

5. Navigate to the mounted folder (which corresponds to `/PS2DEV` inside the container):
   ```bash
   cd /PS2DEV
   ```

6. Clone the GSM repository from GitHub:
   ```bash
   git clone https://github.com/PhilRoll/gsm.git
   ```

7. Change to the `gsm` directory:
   ```bash
   cd gsm
   ```

8. Run the `make` command to compile the source code:
   ```bash
   make
   ```

9. After the build completes, you should see a `build` folder with `PS1_HDMI_FIX.ELF` and `uncompressed_PS1_HDMI_FIX` files inside.

10. You can now run the `.ELF` file on PCSX2 with "Run ELF" or on real hardware with wLaunchElf (remember to use YPbPr output on real hardware).


## Credits
Thanks to [Sestain](https://github.com/sestain) for their fork of GSM.
Thanks to [taka-tuos]() for linking Pre-built toolchain in their fork of GSM.
Thanks to [taka-tuos](https://github.com/taka-tuos) for linking Pre-built toolchain in their fork of GSM.
Thanks to [doctorxyz](https://github.com/doctorxyz), dlanor, [SP193](https://www.psx-place.com/members/sp193.19/), reprep for GSM.
