# PS1 HDMI/Component Fix for PS2

## Description

Play your PS1 games on modern TVs using a PS2 Component cable or HDMI adapter!

- Automatic disc region control (PAL/NTSC)
- Automatic GSM configuration for correct display of PS1 games
- Automatic disc start-up function
- Perfect for playing PS1 games with an HDMI/component cable

## Compiling and Usage

### Windows Compilation

1. To build this (Windows only), you'll need the [pre-built homebrew PlayStation 2 MinGW toolchain](https://github.com/ps2dev/ps2toolchain/releases/tag/2018-10-19).
2. Extract the `.7z` file to `C:/`.
3. Go to `MinGW/msys/1.0` and open `msys.bat`.
4. `cd` into the git-cloned/downloaded folder of this fork.
5. From the source folder, run:
   ```bash
   make
   ```
   This creates a `build` folder containing `PS1_HDMI_FIX.ELF` and `uncompressed_PS1_HDMI_FIX`.
6. Run the `.ELF` file on PCSX2 with "Run ELF", or on real hardware with wLaunchElf. (On real hardware, you must use the YPbPr output setting, or it will crash / display incorrectly.)

---

### Docker Compilation (Windows / macOS)

Alternatively, you can compile the project using a Docker container that provides the PS2 development environment:

1. Make sure Docker is installed and running on your system.
   - You can download Docker from [here](https://www.docker.com/get-started).

2. Pull the PS2DEV Docker image:
   ```bash
   docker pull ps2dev/ps2dev:v1.0
   ```

3. Mount the folder containing the source code and run the Docker container:
   ```bash
   docker run -it -w /PS2DEV -v /path/to/your/source/folder:/PS2DEV --name ps2dev_container ps2dev/ps2dev:v1.0 sh
   ```
   Replace `/path/to/your/source/folder` with the path to the folder where the source code is located.

   **Windows note:** to make the path user-independent (this example creates a `PS2DEV` folder on the desktop, which will hold all the files):
   ```powershell
   docker run -it -w /PS2DEV -v ${env:USERPROFILE}/Desktop/PS2DEV:/PS2DEV --name ps2dev_container ps2dev/ps2dev:v1.0 sh
   ```

4. Once inside the container, install the necessary development tools and libraries:
   ```bash
   apk add build-base cmake git texinfo flex bison gettext gmp-dev mpfr-dev mpc1-dev zlib-dev nano
   ```

5. Navigate to the mounted folder (which corresponds to `/PS2DEV` inside the container):
   ```bash
   cd /PS2DEV
   ```

6. Clone this repository from GitHub:
   ```bash
   git clone https://github.com/PhilRoll/ps1-hdmi-component-fix-ps2.git
   ```

7. Change into the cloned directory:
   ```bash
   cd ps1-hdmi-component-fix-ps2
   ```

8. Run `make` to compile the source code:
   ```bash
   make
   ```

9. After the build completes, you should see a `build` folder containing `PS1_HDMI_FIX.ELF` and `uncompressed_PS1_HDMI_FIX`.

10. Run the `.ELF` file on PCSX2 with "Run ELF", or on real hardware with wLaunchElf (remember to use YPbPr output on real hardware).

---

### Linux Compilation (Ubuntu / Linux Mint / Fedora / Arch)

You can compile the project on Linux using Docker, without installing the PS2 toolchain manually.

1. Make sure Docker is installed and running on your system. If it isn't, install it for your distribution:

   **Ubuntu / Linux Mint:**
   ```bash
   sudo apt update
   sudo apt install docker.io
   sudo systemctl enable --now docker
   ```

   **Fedora:**
   ```bash
   sudo dnf install docker
   sudo systemctl enable --now docker
   ```

   **Arch Linux:**
   ```bash
   sudo pacman -S docker
   sudo systemctl enable --now docker
   ```

2. Navigate to the folder where you want the compiled files to end up:
   ```bash
   cd /path/to/your/folder
   ```

3. Pull the PS2DEV Docker image:
   ```bash
   sudo docker pull ps2dev/ps2dev:v1.0
   ```

4. Run the Docker container, mounting your current folder as `/output`:
   ```bash
   sudo docker run -it --rm -e HOST_UID=$(id -u) -e HOST_GID=$(id -g) -v "$PWD":/output:z --name ps2dev_container ps2dev/ps2dev:v1.0 sh
   ```
   > **Note:** the `:z` suffix is required on distributions using SELinux (e.g. Fedora), otherwise the container won't be able to write to the mounted folder. It's harmless on distributions without SELinux (e.g. Ubuntu, Mint, Arch).

5. Once inside the container, install the necessary development tools and libraries:
   ```bash
   apk add build-base cmake git texinfo flex bison gettext gmp-dev mpfr-dev mpc1-dev zlib-dev nano
   ```

6. Clone this repository from GitHub:
   ```bash
   git clone https://github.com/PhilRoll/ps1-hdmi-component-fix-ps2.git
   ```

7. Change into the cloned directory:
   ```bash
   cd ps1-hdmi-component-fix-ps2
   ```

8. Run `make` to compile the source code:
   ```bash
   make
   ```

9. Copy the compiled files to the mounted output folder, then fix their ownership so they belong to your user instead of root:
   ```bash
   cp -r build /output/
   chown -R $HOST_UID:$HOST_GID /output/build
   ```

10. Exit the container (it will be automatically removed thanks to `--rm`):
    ```bash
    exit
    ```

11. You'll find the compiled files in the `build` folder, in the directory you started from:
    ```
    build/PS1_HDMI_FIX.ELF
    build/uncompressed_PS1_HDMI_FIX.ELF
    ```

12. Run the `.ELF` file on PCSX2 with "Run ELF", or on real hardware with wLaunchElf (remember to use YPbPr output on real hardware).

#### Optional: cleaning up the Docker image

If you're done and want to free up disk space:
```bash
sudo docker rmi ps2dev/ps2dev:v1.0
```
Verify nothing related is left:
```bash
sudo docker ps -a --filter "name=ps2dev"
sudo docker images --filter "reference=ps2dev*"
```

## Credits

Thanks to:

- [Sestain](https://github.com/sestain) for his GSM fork.
- [SP193](https://www.psx-place.com/members/sp193.19/) and reprep for GSM and the PS1VModeNeg project.
- [taka-tuos](https://github.com/taka-tuos) for linking the pre-built toolchain in their fork of GSM.
- Everyone who supported me in the "PS2 Scene" Discord group.
