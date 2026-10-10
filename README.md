# AdobeWine

**Adobe Creative Cloud apps on Linux, through a patched Wine.**

AdobeWine is Wine 11.18 plus 59 patches. With it, these apps install through
Creative Cloud, sign in with a normal Adobe account and run with GPU acceleration:
Adobe Photoshop, Illustrator, Premiere Pro, Audition, Media Encoder and the
Creative Cloud desktop app.

> **Status: early testing (0.1.0).** It works well on the reference machine (Arch-based,
> NVIDIA). It has not yet been tested on other machines, which is where you come in: please
> [try it and report back](CONTRIBUTING.md), good or bad.

| App (2026 versions) | State on the reference machine |
|---|---|
| Creative Cloud desktop 6.10 | Sign in, Home, Apps (install, update, open, uninstall), Fonts, Files |
| Photoshop 27.10 | Home, New, layers, masks, type, filters, Remove Background, Generative Fill, export |
| Illustrator 30.8 | Home, New, shapes, pen, type, effects, Pathfinder, Image Trace, save/export AI/PDF/SVG/PNG/JPEG |
| Premiere Pro 26.5 | Import, timeline editing, Lumetri, titles, playback, NVENC export |
| Media Encoder 26.5 | Queue, presets, CUDA renderer, NVENC/NVDEC export |
| Audition 26.5 | Recording, multitrack, effects, noise reduction, mixdown, video track |

Exports were checked against the same exports made on Windows. For example, Photoshop PNG
output is pixel-identical, Audition's normalised audio is bit-identical, and Premiere's 4K
H.264 export took 9.9 s here against 9.1 s on Windows. Details are in
[docs/STATUS.md](docs/STATUS.md).

## Why this exists

I'm not a Wine developer. I needed Adobe's apps for my work and didn't want to keep Windows
just for them, so I built this for my own laptop, with a lot of help from Claude, an AI
coding assistant. I'm sharing it because many people want to get off both Windows and
macOS, keep working with Adobe, and still tinker with their own machines. It has only been
tested on one machine so far. Your reports, fixes and criticism are what will make it work
for everyone else.

## What it is and what it is not

- It **is** a Wine build: open-source patches on top of upstream Wine (LGPL-2.1-or-later),
  plus scripts that set up a Wine prefix with DXVK, vkd3d-proton and nvidia-libs.
- It **does not** include or download any Adobe software. You install Adobe's apps yourself
  with Adobe's own Creative Cloud installer.
- It **does not** crack, bypass or modify Adobe licensing. You need your own Creative
  Cloud subscription and you sign in normally.
- It is **not affiliated with, endorsed by or supported by Adobe**. "Adobe", "Photoshop",
  "Illustrator", "Premiere Pro", "Audition" and "Creative Cloud" are trademarks of Adobe
  Inc., used here only to say which programs this Wine build is meant to run.

## Install

- **Arch Linux and derivatives (CachyOS, EndeavourOS, Manjaro, Garuda):** the AUR packages
  `adobewine` / `adobewine-bin` are being submitted. Until then, build the same package from
  this repository (see [docs/INSTALL-ARCH.md](docs/INSTALL-ARCH.md)).
- **Gentoo:** Portage packages in the `::snakebyte` overlay (`app-emulation/adobewine`,
  maintained outside this project). See [docs/INSTALL-GENTOO.md](docs/INSTALL-GENTOO.md).
- **Any other distribution:** build from source. See [docs/INSTALL.md](docs/INSTALL.md).

Once installed, the short version is:

```sh
adobewine setup --windows-fonts /path/to/Windows/Fonts  # create the prefix (once)
adobewine run ~/Downloads/Creative_Cloud_Set-Up.exe       # install Creative Cloud, sign in
adobewine fix-signin                                      # if the sign-in window stays blank
adobewine cc                                              # install apps from Creative Cloud
adobewine launchers                                       # add them to your app menu
```

Which distributions, GPUs and desktops should work: [docs/COMPATIBILITY.md](docs/COMPATIBILITY.md).
When something breaks: [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md).

## How it was made

The patches come from comparing the same Adobe workflows on real Windows and under Wine.
API traces, window trees and output files were recorded on Windows, and each difference
was fixed in Wine with a small test program. Every patch has a one-line description in
[docs/PATCHES.md](docs/PATCHES.md), and [docs/HOW-IT-WORKS.md](docs/HOW-IT-WORKS.md)
describes the method. Patches 0001-0003 come from
[PhialsBasement](https://github.com/PhialsBasement)'s Adobe installer work for Wine. The
rest were written for this project, and we would like to see them upstream in WineHQ.

## Help wanted

Where this is going and what's next: [docs/ROADMAP.md](docs/ROADMAP.md)
([comments welcome](../../discussions/9)).

- **Test it** on your distribution and GPU, and tell us in a
  [compatibility report](../../issues/new?template=compatibility.yml).
- **Report bugs** with a log: `adobewine` prints where it saved each run's log.
- **Patches welcome**, especially for AMD and Intel GPUs, other distributions, and
  upstreaming. See [CONTRIBUTING.md](CONTRIBUTING.md).
- Questions and ideas: [Discussions](../../discussions).

## License

The patches and scripts are licensed under the GNU LGPL 2.1 or later, the same as Wine
(see [LICENSE](LICENSE)). DXVK, vkd3d-proton, nvidia-libs, wine-gecko and wine-mono are
downloaded from their own projects and keep their own licenses.
