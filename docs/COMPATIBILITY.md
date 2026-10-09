# Compatibility review

What AdobeWine needs from the system, what has been tested, and what is expected to work
on other Linux distributions and Unix-like systems. "Tested" means checked on real
hardware; everything else is reasoned from the dependencies and **needs reports from
users**: please open a [compatibility report](../../../issues/new?template=compatibility.yml).

## Reference system (tested)

All results in [STATUS.md](STATUS.md) come from this laptop:

| Part | Version |
|---|---|
| Machine | Samsung laptop, Intel Core i9-13900H, 30 GB RAM |
| GPUs | Intel Iris Xe (Raptor Lake-P, drives the screen) + NVIDIA GeForce RTX 4070 Laptop (AD106M, renders) |
| Distribution | CachyOS (Arch-based), rolling, October 2026 |
| Kernel | 7.2.8-cachyos |
| NVIDIA driver | 615.71.09, open kernel module (`nvidia-open`), Vulkan 1.4.351 |
| Mesa (Intel) | 26.2.3, Vulkan 1.4.354 (ANV) |
| Vulkan loader | 1.4.357 |
| Desktop | KDE Plasma 6.7.5 on Wayland (KWin 6.7.5); Wine through XWayland 24.1.13 |
| GStreamer | 1.28.7 (Media Foundation playback) |
| winetricks | 20260125 |
| Wine base | 11.18 + 53 patches; DXVK 3.1.1, vkd3d-proton 3.0.1, nvidia-libs 1.0.2 |

GPU work (canvas, video effects, export) runs on the RTX 4070 through PRIME render
offload. CUDA (Premiere's Mercury engine, Media Encoder) and NVENC/NVDEC (hardware
H.264/HEVC) work through nvidia-libs.

## What the system must provide

| Requirement | Why | Notes |
|---|---|---|
| x86-64 CPU | Adobe ships x86-64 Windows binaries only | ARM (Apple Silicon, Snapdragon, Raspberry Pi) is not possible without an x86 emulator |
| Linux kernel 5.16+ | Wine 11 (futex2, `/dev/ntsync` optional on 6.14+) | |
| glibc | Wine and the GPU drivers | musl distributions (Alpine, Void-musl) are not supported by Wine's build |
| Vulkan 1.3 driver | DXVK 3 and vkd3d-proton 3 need Vulkan 1.3 | see GPUs below |
| X11 or XWayland | the patches target Wine's X11 driver | Wine's own Wayland driver is not used |
| GStreamer 1.x + plugins-good | video import/playback through Media Foundation | |
| ~25 GB disk | Wine build + prefix + Adobe apps | |
| 16 GB RAM (32 GB recommended) | Adobe's own recommendation | |

## GPUs

| GPU | Expected | Notes |
|---|---|---|
| NVIDIA RTX 20/30/40/50, proprietary or open driver 550+ | **works (tested on RTX 4070)** | CUDA and NVENC available to Premiere and Media Encoder |
| NVIDIA GTX 900/1000/1600 | should work | older Vulkan feature levels; CUDA works on Pascal+ |
| NVIDIA with nouveau/NVK | untested | NVK Vulkan is improving; no CUDA/NVENC |
| AMD RDNA 1-4 (RADV) | untested, expected to work for UI and GPU canvas | no CUDA: Premiere falls back to software rendering or OpenCL; no AMF hardware encoding through Wine |
| AMD GCN 3-5 (RADV) | untested | |
| Intel Arc / Xe (ANV) | untested | Iris Xe tested only as display GPU; no Quick Sync through Wine |
| Hybrid Intel/AMD + NVIDIA | **works (tested)** | AdobeWine enables PRIME offload automatically |

## Community reports

Results from other people's machines. Thank you! Add yours with a
[compatibility report](../../../issues/new?template=compatibility.yml).

| Report | Distribution | Hardware | Desktop | Apps | Result |
|---|---|---|---|---|---|
| [#2](../../../issues/2) | Linux Mint 22.3 (kernel 7.0, built from source) | i7-12700H, 16 GB, Iris Xe + RTX 4060 Laptop, NVIDIA 595.91 | Cinnamon 6.6, X11 | Illustrator 2026 | works well. The Creative Cloud web installer showed a white window (the offline installer worked); high memory use with 16 GB RAM and no swap, one freeze (being investigated); Creative Cloud's close button does not quit it (File > Exit does) |
| [#2](../../../issues/2#issuecomment-6056733418) (NickPittas) | Arch-based (Omarchy), kernel 7.2.5 | RTX 4090, NVIDIA 610.57.04 | Wayland | Creative Cloud web installer 2.14.0.82 | white window; NickPittas traced it to three missing browser features in Wine (fix in progress: patches 0055-0057; until then the launcher works around it) |

## Tested in containers

The release tarball (`adobewine-0.1.0-x86_64.tar.zst`) was installed in clean containers
of each distribution, with exactly the runtime dependencies listed in
[INSTALL.md](INSTALL.md) / [INSTALL-ARCH.md](INSTALL-ARCH.md), and run as a normal user.
Run it yourself with `tests/distro/check.sh <tarball> [image ...]` (Docker or Podman).
This checks installation and that Wine works; it cannot check graphics or the Adobe apps,
which need a real GPU and desktop.

| Distribution (glibc) | Documented deps install | Wine starts | Prefix created | 64-bit / 32-bit (WoW64) programs | Libraries missing |
|---|---|---|---|---|---|
| Arch Linux, Oct 2026 (2.44) | yes | yes | yes | yes / yes | none that matter (`libpcsclite`: smart cards only) |
| Debian 13 trixie (2.41) | yes, after enabling `contrib` | yes | yes | yes / yes | FFmpeg 63 (`winedmo`, see below), `libpcap` (packet capture only) |
| Ubuntu 24.04 LTS (2.39) | yes | yes | yes | yes / yes | FFmpeg 63, `libpcap` |
| Fedora 42 (2.41) | yes | yes | yes | yes / yes | FFmpeg 63 |
| openSUSE Tumbleweed (2.44) | yes | yes | yes | yes / yes | FFmpeg 63 |

FFmpeg: the prebuilt runtime is built on Arch and links Wine's FFmpeg media decoder
(`winedmo.so`) against libavcodec 63. Where that is missing, Wine uses its GStreamer path
for video import. Building from source on your distribution links against its own FFmpeg.
Ubuntu-based (Linux Mint, Pop!_OS, Zorin) and Fedora-based (Nobara, Ultramarine) systems
use the same packages, but were not tested separately.

## Linux distributions

| Distribution | Install route | Status |
|---|---|---|
| CachyOS | AUR (`adobewine-bin` / `adobewine`) | **tested** |
| Arch Linux, EndeavourOS, Garuda | AUR | Arch: runtime tested in a container; desktop use expected to work (same packages) |
| Manjaro | AUR | expected to work; Manjaro's repositories lag Arch by a few weeks, so build dependencies may be older |
| Fedora 41+ | build from source, or prebuilt | prebuilt runtime tested in a container (Fedora 42); desktop use expected to work; `dnf builddep wine` provides the build dependencies |
| Debian 13, Ubuntu 24.04+, Linux Mint 22+, Pop!_OS | build from source, or prebuilt | prebuilt runtime tested in containers (Debian 13, Ubuntu 24.04); desktop use expected to work; needs `deb-src` enabled for `apt build-dep wine`; Mesa from the distribution may be older (use the kisak/oibaf PPA on Ubuntu for newer RADV/ANV) |
| openSUSE Tumbleweed | build from source, or prebuilt | prebuilt runtime tested in a container; desktop use expected to work |
| openSUSE Leap, Debian 12, Ubuntu 22.04 | build from source | unlikely without newer MinGW/Vulkan headers; not recommended |
| Gentoo | build from source | expected to work with the Wine USE flags for Vulkan, X, GStreamer |
| NixOS | build from source in a `nix-shell` with Wine's build inputs | untested; a Nix derivation would be welcome |
| Fedora Silverblue/Kinoite, Bazzite, SteamOS (Steam Deck) | build and run inside a Distrobox (Arch or Fedora image) | untested; the prefix in `~/.local/share` is shared with the host |
| Alpine, Void-musl, Chimera | not supported | musl |

## Other Unix-like systems

| System | Status | Why |
|---|---|---|
| FreeBSD | not supported yet | Wine runs on FreeBSD, but DXVK, vkd3d-proton and nvidia-libs target Linux, and several patches touch Linux-only paths (X11 client surfaces with Vulkan offload, ntdll). A port is possible; volunteers welcome |
| OpenBSD, NetBSD | not supported | no maintained Wine with WoW64 and Vulkan |
| macOS | not supported | Wine's macOS driver and MoltenVK are separate code paths; Adobe has native macOS versions anyway |
| Windows Subsystem for Linux | not useful | Adobe runs natively on Windows |
| ChromeOS (Crostini) | not supported | no Vulkan GPU access for Linux apps on most devices |

## Desktops and display servers

| Desktop | Status |
|---|---|
| KDE Plasma 6 Wayland (XWayland) | **tested** |
| KDE Plasma 6 X11 | expected to work |
| GNOME 46+ (Wayland/XWayland or X11) | untested |
| Xfce, Cinnamon, MATE (X11) | untested, expected to work |
| Hyprland, Sway, niri (XWayland) | untested; tiling layouts may fight Adobe's floating panels |

Wine windows are created without window-manager decorations (Adobe apps draw their own
title bars). HiDPI: the reference machine runs at 100 % scale; fractional scaling under
XWayland is untested.

## Adobe app versions

The patches were developed against the 2026 releases: Photoshop 27.10, Illustrator
30.8.2, Premiere Pro 26.5.2, Audition 26.5, Media Encoder 26.5.2, Creative Cloud
desktop 6.10. Newer versions install through Creative Cloud as usual; please report when
an update breaks something. Other Adobe apps (After Effects, InDesign, Lightroom
Classic, Bridge) are untested apart from Bridge installing and opening.
