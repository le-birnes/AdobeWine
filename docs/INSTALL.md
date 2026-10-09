# Installing AdobeWine

This guide is for any Linux distribution. On Arch Linux or an Arch-based distribution
(CachyOS, EndeavourOS, Manjaro, Garuda), use the AUR package instead:
[INSTALL-ARCH.md](INSTALL-ARCH.md).

You need:

- a 64-bit x86 PC with a **Vulkan-capable GPU and driver** (NVIDIA proprietary driver,
  or Mesa RADV / ANV for AMD / Intel), see [COMPATIBILITY.md](COMPATIBILITY.md);
- an **Adobe Creative Cloud subscription** (AdobeWine contains no Adobe software);
- about **25 GB** of free disk space (Wine build ~8 GB, each Adobe app 3-10 GB);
- for the best results, **fonts from your own Windows installation** (a Windows
  partition, or a copy of `C:\Windows\Fonts`). Adobe's interface uses Segoe UI, and
  Photoshop's menu bar crashes without it. Fonts are not included because they are
  Microsoft's and cannot be redistributed.

Nothing is installed system-wide. The Wine build, the downloads and the Windows prefix all
live in `~/.local/share/adobewine` (change it with the `ADOBEWINE_HOME` variable).

## 1. Install the build tools

The build needs the usual Wine build dependencies, a MinGW cross-compiler, git and
winetricks. The commands below are the standard ones for each family. **Only the Arch
route has been tested so far**; please report how these go.

**Debian 13 / Ubuntu 24.04 or newer** (enable "source code" repositories first):
```sh
sudo apt build-dep wine
sudo apt install git ccache mingw-w64 winetricks curl zstd xz-utils \
     gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-libav
```

**Fedora 41 or newer:**
```sh
sudo dnf builddep wine
sudo dnf install git ccache mingw64-gcc mingw32-gcc winetricks curl zstd \
     gstreamer1-plugins-good gstreamer1-plugins-bad-free
```

**openSUSE Tumbleweed:**
```sh
sudo zypper source-install --build-deps-only wine
sudo zypper install git ccache mingw64-cross-gcc mingw32-cross-gcc winetricks curl zstd \
     gstreamer-plugins-good gstreamer-plugins-bad
```

### Runtime dependencies only

If you use a prebuilt runtime (see "Prebuilt runtime" below) you only need what AdobeWine
needs to *run*:

```sh
# Debian 13: winetricks is in the "contrib" section; enable it first:
sudo sed -i 's/^Components: main$/Components: main contrib/' /etc/apt/sources.list.d/debian.sources
sudo apt update
# Debian / Ubuntu (Ubuntu has winetricks in "universe", enabled by default)
sudo apt install winetricks curl zstd libvulkan1 libgnutls30 libpulse0 libasound2t64 \
     gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
     gstreamer1.0-libav libfreetype6 libfontconfig1 libunwind8 libxkbcommon0 p7zip-full python3-pil
# Fedora
sudo dnf install winetricks curl zstd vulkan-loader gnutls pulseaudio-libs alsa-lib \
     gstreamer1-plugins-base gstreamer1-plugins-good gstreamer1-plugins-bad-free \
     freetype fontconfig libunwind libxkbcommon p7zip python3-pillow
# openSUSE Tumbleweed
sudo zypper install winetricks curl zstd libvulkan1 libgnutls30 libpulse0 libasound2 \
     gstreamer-plugins-base gstreamer-plugins-good gstreamer-plugins-bad \
     libfreetype6 fontconfig libunwind8 libxkbcommon0 7zip python313-Pillow
# Arch: see INSTALL-ARCH.md, section 1.2
```

These lines are checked automatically in clean containers (`tests/distro/check.sh`), see
[COMPATIBILITY.md](COMPATIBILITY.md#tested-in-containers).

### Prebuilt runtime (experimental outside Arch)

Each release has `adobewine-<version>-x86_64.tar.zst`, built on Arch Linux. It contains
`/opt/adobewine` (the Wine runtime plus the scripts). On other distributions with a
recent glibc it may work:

```sh
sudo tar -C / -xf adobewine-0.1.0-x86_64.tar.zst
sudo ln -sf /opt/adobewine/bin/adobewine /usr/local/bin/adobewine
```

This has been tested on Debian 13, Ubuntu 24.04, Fedora 42 and openSUSE Tumbleweed: Wine
starts, creates a prefix and runs 64-bit and 32-bit programs. One known limit: the build
links Wine's FFmpeg media decoder (`winedmo`) against Arch's current FFmpeg (libavcodec 63),
which those distributions do not ship yet, so video import uses Wine's GStreamer path instead. For video
work outside Arch, building from source (below) is the better choice.

If `adobewine setup` fails with missing-library or glibc errors, build from source instead.

## 2. Get AdobeWine and build it

```sh
git clone https://github.com/le-birnes/AdobeWine.git
cd AdobeWine
scripts/build-wine.sh          # 20-60 minutes the first time
```

The script downloads Wine 11.18, applies the patches in `patches/` and installs the
result into `~/.local/share/adobewine/runtime`. Later runs are much faster when
`ccache` is installed.

Put the `adobewine` command on your PATH:

```sh
mkdir -p ~/.local/bin && ln -sf "$PWD/bin/adobewine" ~/.local/bin/adobewine
```

## 3. Create the prefix

```sh
adobewine setup --windows-fonts /mnt/windows/Windows/Fonts
```

This downloads DXVK, vkd3d-proton and (on NVIDIA) nvidia-libs, checks their checksums,
installs wine-gecko, wine-mono, Microsoft's shader compiler and the core fonts through
winetricks, and applies the settings the Adobe apps need. Leave out `--windows-fonts` if
you have no Windows fonts at hand; you can run the command again later with it.

## 4. Install Creative Cloud and sign in

Download the Creative Cloud installer from Adobe
(<https://www.adobe.com/download/creative-cloud>), then:

```sh
adobewine run ~/Downloads/Creative_Cloud_Set-Up.exe
```

For the web installer 2.14.0.82, which otherwise shows a white window (issue #2),
`adobewine run` adds a small fix to the installer's page while it runs; this needs
`inotify-tools` installed. If the installer window still stays white, use Adobe's
Creative Cloud offline installer instead (see [TROUBLESHOOTING.md](TROUBLESHOOTING.md#installation-and-sign-in)).

Sign in with your Adobe account when the installer asks. If the sign-in window stays
blank or white, close it and run:

```sh
adobewine fix-signin
adobewine cc
```

`fix-signin` makes Adobe's sign-in library use its Internet Explorer-based window instead
of the Chromium-based one, which does not draw under Wine. It does not change anything
about your account or licence.

## 5. Install and start the apps

In the Creative Cloud window, open **Apps** and install the apps you want (Photoshop,
Illustrator, Premiere Pro, Audition, Media Encoder). Then:

```sh
adobewine launchers        # adds them to your desktop's application menu
adobewine photoshop        # or illustrator, premiere, audition, encoder, cc
```

To close everything cleanly (the apps may ask to save): `adobewine off`.

## Updating AdobeWine

```sh
cd AdobeWine && git pull && scripts/build-wine.sh
```

Your prefix and installed apps stay as they are. Run `adobewine setup` again if the release
notes say so.

## Uninstalling

```sh
adobewine off
adobewine launchers --remove
rm -rf ~/.local/share/adobewine     # removes the prefix, including the installed Adobe apps
rm ~/.local/bin/adobewine
```
