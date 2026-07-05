# PS1 HDMI/Component Fix for PS2

Play your PS1 games on modern TVs using a PS2 with Component cable or HDMI adapter!

## Features

- Automatic disc region control (PAL/NTSC)
- Automatic GSM configuration for correct display of PS1 games
- Automatic disc start-up function
- Perfect for playing PS1 games with an HDMI/component cable

## Building

There are two ways to build this project: natively on Windows, or with Docker on any platform (Windows, macOS, Linux).

### Option A — Docker (Windows / macOS / Linux)

Recommended if you don't want to install the PS2 toolchain manually.

**1. Install Docker**, if you don't have it already:
- **Windows / macOS:** [Docker Desktop](https://www.docker.com/get-started).
- **Linux (Ubuntu / Linux Mint):**
  ```bash
  sudo apt update && sudo apt install docker.io
  sudo systemctl enable --now docker
  ```
- **Linux (Fedora):**
  ```bash
  sudo dnf install docker
  sudo systemctl enable --now docker
  ```
- **Linux (Arch):**
  ```bash
  sudo pacman -S docker
  sudo systemctl enable --now docker
  ```

**2. Navigate to the folder** where you want the compiled files to end up:
```bash
cd /path/to/your/folder        # macOS / Linux / Git Bash on Windows
```
```powershell
cd C:\path\to\your\folder      # Windows PowerShell
```

**3. Pull the PS2DEV image:**
```bash
docker pull ps2dev/ps2dev:v1.0
```
> On Linux, prepend `sudo` to every `docker` command, unless your user is already in the `docker` group.

**4. Start the container**, mounting the current folder as `/output`:
```bash
docker run -it --rm -e HOST_UID=$(id -u) -e HOST_GID=$(id -g) -v "$PWD":/output:z --name ps2dev_container ps2dev/ps2dev:v1.0 sh
```
```powershell
docker run -it --rm -v ${PWD}:/output:z --name ps2dev_container ps2dev/ps2dev:v1.0 sh   # Windows PowerShell
```
> The `:z` suffix is required on Linux distributions using SELinux (e.g. Fedora). It's harmless everywhere else.

**5. Inside the container**, install dependencies, clone, and build:
```bash
apk add build-base cmake git texinfo flex bison gettext gmp-dev mpfr-dev mpc1-dev zlib-dev nano
git clone https://github.com/PhilRoll/ps1-hdmi-component-fix-ps2.git
cd ps1-hdmi-component-fix-ps2
make
```

**6. Copy the build output** to the mounted folder:
```bash
cp -r build /output/
```
**Linux only** — fix file ownership so they belong to your user instead of root:
```bash
chown -R $HOST_UID:$HOST_GID /output/build
```

**7. Exit the container** (it's removed automatically thanks to `--rm`):
```bash
exit
```

You'll now find `build/PS1_HDMI_FIX.ELF` and `build/uncompressed_PS1_HDMI_FIX.ELF` in the folder you started from.

<details>
<summary>Optional: remove the Docker image afterwards</summary>

```bash
docker rmi ps2dev/ps2dev:v1.0
docker ps -a --filter "name=ps2dev"
docker images --filter "reference=ps2dev*"
```
(on Linux, prepend `sudo` as usual)
</details>

### Option B — Native (Windows only, MinGW toolchain)

1. Download the [pre-built homebrew PlayStation 2 MinGW toolchain](https://github.com/ps2dev/ps2toolchain/releases/tag/2018-10-19) and extract the `.7z` file to `C:/`.
2. Go to `MinGW/msys/1.0` and open `msys.bat`.
3. `cd` into this repository's folder.
4. Run:
   ```bash
   make
   ```

You'll now find `build/PS1_HDMI_FIX.ELF` and `build/uncompressed_PS1_HDMI_FIX.ELF` in the repository folder.

## Usage

Once you have `PS1_HDMI_FIX.ELF` (from either build method above):

- **On PCSX2:** use "Run ELF".
- **On real hardware:** run it with wLaunchElf. You must set the **YPbPr** output setting, or the game will crash or display incorrectly.

## Credits

Thanks to:

- [Sestain](https://github.com/sestain) for his GSM fork.
- [SP193](https://www.psx-place.com/members/sp193.19/) and reprep for GSM and the PS1VModeNeg project.
- [taka-tuos](https://github.com/taka-tuos) for linking the pre-built toolchain in their fork of GSM.
- Everyone who supported me in the "PS2 Scene" Discord group.
