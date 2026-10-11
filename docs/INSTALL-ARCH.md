# Installing AdobeWine on Arch Linux

Detailed procedure for Arch Linux and Arch-based distributions (CachyOS, EndeavourOS,
Manjaro, Garuda, ArcoLinux). This is the tested route: the reference machine runs CachyOS
(see [COMPATIBILITY.md](COMPATIBILITY.md)).

There are two AUR packages:

| Package | What it does | Time to install |
|---|---|---|
| `adobewine-bin` | prebuilt Wine runtime from the GitHub release | about 1 minute |
| `adobewine` | builds Wine 11.18 + the patches on your machine | 20-60 minutes |

Both install the same files to `/opt/adobewine` and the `adobewine` command to `/usr/bin`.
Your prefix and the Adobe apps live in your home folder (`~/.local/share/adobewine`), not
in the package.

## 1. Prepare the system

### 1.1 GPU driver and Vulkan

AdobeWine renders through Vulkan (DXVK, vkd3d-proton), so a working Vulkan driver is
required:

| GPU | Packages |
|---|---|
| NVIDIA (GTX 900 series or newer) | `nvidia-open` (or `nvidia`, or `nvidia-open-dkms` for custom kernels) + `nvidia-utils` |
| AMD (GCN 3 or newer) | `vulkan-radeon` (Mesa RADV) |
| Intel (Gen 9 / Skylake or newer) | `vulkan-intel` (Mesa ANV) |

Check it with `vulkaninfo --summary` (package `vulkan-tools`): your GPU must be listed.

On **hybrid laptops** (Intel or AMD integrated GPU + NVIDIA), AdobeWine detects the
NVIDIA driver and renders on the NVIDIA GPU (PRIME render offload). To force or disable
that, set `ADOBEWINE_NVIDIA_OFFLOAD=1` or `=0`.

### 1.2 Only the runtime dependencies

To install everything AdobeWine needs to *run*, without installing AdobeWine itself (for
example to prepare a machine, or to use a self-built runtime):

```sh
sudo pacman -S --needed desktop-file-utils fontconfig freetype2 gettext glib2 libpcap \
    libunwind libx11 libxcursor libxext libxkbcommon libxi libxrandr wayland \
    vulkan-icd-loader gnutls alsa-lib libpulse gst-plugins-base-libs winetricks curl \
    gst-plugins-good gst-plugins-bad gst-libav p7zip python-pillow
```

These are the packages `adobewine-bin` depends on, plus the recommended optional ones
(media formats and menu icons).

### 1.3 Only the build dependencies

To build the runtime yourself (`scripts/build-wine.sh` from a git checkout):

```sh
sudo pacman -S --needed base-devel git ccache mingw-w64-gcc ffmpeg libcups libgphoto2 \
    libxcomposite libxinerama libxxf86vm mesa opencl-headers opencl-icd-loader pcsclite \
    perl samba sane sdl2 unixodbc v4l-utils vulkan-headers
```

`yay -S adobewine` installs these by itself while building, and `makepkg -so` in a
checkout of the AUR package installs them without building anything.

## 2. Install AdobeWine

> **AUR submission pending.** Until `adobewine` / `adobewine-bin` appear on the AUR, build
> the same package from this repository:
>
> ```sh
> git clone https://github.com/le-birnes/AdobeWine.git
> cd AdobeWine/packaging/aur/adobewine && makepkg -si
> ```
>
> or the prebuilt one (downloads the runtime from the
> [latest release](https://github.com/le-birnes/AdobeWine/releases/latest), currently 0.2.2):
> `cd AdobeWine/packaging/aur/adobewine-bin && makepkg -si`.

Once on the AUR, with an AUR helper (`yay` or `paru`):

```sh
yay -S adobewine-bin        # prebuilt (fast)
# or
yay -S adobewine            # build from source
```

Without an AUR helper:

```sh
git clone https://aur.archlinux.org/adobewine-bin.git
cd adobewine-bin && makepkg -si
```

## 3. Create your prefix

Mount your Windows partition (or copy its `Windows/Fonts` folder somewhere), then:

```sh
adobewine setup --windows-fonts /mnt/windows/Windows/Fonts
```

Without Windows fonts the apps still start, but Adobe's interface falls back to other
fonts and Photoshop's menu bar can crash. You can add the fonts later by running the same
command again.

The setup downloads DXVK 3.1.1, vkd3d-proton 3.0.1 and, on NVIDIA, nvidia-libs 1.0.2
(checksums verified), then installs wine-gecko, wine-mono, `d3dcompiler_47`,
`corefonts` and `tahoma` through winetricks. It takes 5-10 minutes.

## 4. Creative Cloud, sign-in, apps

```sh
# installer from https://www.adobe.com/download/creative-cloud
adobewine run ~/Downloads/Creative_Cloud_Set-Up.exe
```

1. Sign in with your Adobe account when asked.
2. If the sign-in window is blank: close it, run `adobewine fix-signin`, then `adobewine cc`.
3. In Creative Cloud, go to **Apps** and install Photoshop, Illustrator, Premiere Pro,
   Audition or Media Encoder.
4. Add them to your application menu: `adobewine launchers`.

Start apps from the menu or from a terminal:

```sh
adobewine photoshop ~/Pictures/photo.jpg
adobewine illustrator
adobewine premiere
```

Logs go to `~/.local/share/adobewine/logs/`. `adobewine off` closes everything cleanly.

## 5. Desktop notes

- **KDE Plasma (Wayland or X11)**: tested. On Wayland, Wine runs through XWayland
  (install `xorg-xwayland`, normally already present).
- **GNOME, Xfce, Hyprland, Sway**: untested; they should work through XWayland. Please
  report.
- AdobeWine turns off window-manager decorations for Wine windows, because Adobe apps
  draw their own title bar. Use the app's own buttons or `Alt+F4` to close.

## 6. Updating and removing

```sh
yay -Syu                       # updates adobewine / adobewine-bin with the system
adobewine setup                # only when the release notes say so
```

To remove:

```sh
adobewine off
adobewine launchers --remove
sudo pacman -Rns adobewine-bin    # or adobewine
rm -rf ~/.local/share/adobewine   # your prefix and the Adobe apps in it
```

## Arch-specific tips

- **CachyOS / custom kernels**: use the matching NVIDIA module package
  (`linux-cachyos-nvidia-open` or `nvidia-open-dkms`).
- **ntsync**: Wine 11 uses `/dev/ntsync` when the kernel module is loaded
  (`sudo modprobe ntsync`; Arch's `ntsync-autoload` package loads it at boot). It is
  optional.
- **Multilib is not needed**: AdobeWine is built in Wine's new WoW64 mode, so 32-bit parts
  run without 32-bit Linux libraries.
