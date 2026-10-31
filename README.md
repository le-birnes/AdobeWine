# AdobeWine

**Run Adobe Creative Cloud on Linux: Photoshop, Illustrator, Premiere Pro, Lightroom, Acrobat and more.**

Hi, and welcome! Thanks for stopping by.

AdobeWine is for people who would like to try Linux but need Adobe's apps for their work. You install it, sign
in with your own Adobe account, and use the apps as usual, with your graphics card doing the heavy lifting.

It's made for people who just want to enjoy their software: one package to install, no Wine knowledge needed.
AI was used as a tool to build it, and we say so plainly: see [Why this exists](#why-this-exists).

**See it in action:** [a short showreel (1:24)](https://youtu.be/3Vbzt4cGSQQ), made entirely in these apps on
Linux, from the painting to the final export.

> **Still early (0.5.0).** AdobeWine is built and tested day to day on one Arch-based laptop (CachyOS, NVIDIA RTX
> 4070). The installer also runs on Fedora, Ubuntu, Debian, Linux Mint, openSUSE and others, but those have had less
> real-world use. If you try it, please tell us how it went, good or bad: it really helps.

## Install it (about 2 minutes, plus Adobe's download)

You need: a Linux computer with a graphics card from the last few years, about 25 GB of free space, and an Adobe
Creative Cloud subscription.

1. **Download** `CCINSTALLER.flatpak` from the [releases page](../../releases) (the newest is at the top).
2. **Double-click** the file. Your app store opens (GNOME Software, KDE Discover or Mint's Software Manager).
   Click **Install**.
3. **Open AdobeWine** from your app menu. It checks your computer, sets everything up, downloads Creative Cloud
   from Adobe and starts it. This takes about a minute.
4. **Sign in** with your Adobe account in the window that appears.
5. **Install your apps** (Photoshop, Premiere Pro…) from Creative Cloud, as you would on Windows or a Mac.
   They show up in the AdobeWine window and in your app menu.

**Nothing happens when you double-click?** Your system doesn't have Flatpak yet (Ubuntu and Debian don't by
default). Open a terminal, type this once, then try again:

```sh
sudo apt install flatpak gnome-software-plugin-flatpak
```

**Something not working?** Click **Report a problem** in the AdobeWine window. It shows you the report before
anything is sent. More help: [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md).

## What works

| App | State |
|---|---|
| Creative Cloud | Sign in, install, update and open apps, fonts, files |
| Photoshop | Everyday editing, layers, type, filters, Remove Background, Generative Fill, export. All 75 filters and adjustments tested give the same result as on Windows |
| Illustrator | Shapes, pen, type, effects, Pathfinder, Image Trace, save and export |
| Premiere Pro | Import, editing, color (Lumetri), titles, playback, fast NVIDIA export |
| Media Encoder | Export queue, presets, GPU-accelerated encoding |
| Audition | Recording, multitrack, effects, noise reduction, mixdown |
| After Effects | Opens and renders real projects, 3D text; less tested |
| InDesign | Layout, styles, tables, placed images, PDF export; less tested |
| Lightroom Classic | Import, ratings, keywords, collections, Develop, AI masking, export to JPEG, TIFF, PSD and DNG |
| Lightroom | Opens, edits and exports; less tested |
| Bridge | Browsing, previews, metadata, batch rename, Camera Raw, Image Processor, contact sheets |
| Acrobat Pro | Read, comment, edit, combine, export to Word, Excel and PowerPoint, sign, OCR, print. Protected Mode is off |

Details for each app: [docs/STATUS.md](docs/STATUS.md).

## Good to know

- **You need your own Adobe subscription.** AdobeWine contains no Adobe software and doesn't crack or bypass
  anything: you download Creative Cloud from Adobe and sign in normally.
- **NVIDIA is the best-tested graphics card.** Intel graphics start fine in our tests. AMD has fewer reports so
  far: Photoshop works, Premiere is being tested. NVIDIA-only features (CUDA, NVENC) aren't available on AMD or
  Intel, just like on Windows.
- **After Effects can use a lot of memory** while previewing Cinema 4D scenes. Turning off *Cache Frames When
  Idle* and *Multi-Frame Rendering* keeps it in check.
- **Not affiliated with Adobe.** "Adobe", "Photoshop", "Illustrator", "Premiere Pro", "Audition" and "Creative
  Cloud" are trademarks of Adobe Inc., used here only to say which programs AdobeWine runs.

## Other ways to install

- **Arch Linux, CachyOS, EndeavourOS, Manjaro, Garuda:** one command, on the platform AdobeWine is developed on:

  ```sh
  sudo pacman -U https://github.com/le-birnes/AdobeWine/releases/download/v0.5.0/adobewine-0.5.0-1-x86_64.pkg.tar.zst
  adobewine setup
  ```

  Then see [docs/INSTALL-ARCH.md](docs/INSTALL-ARCH.md). The AUR packages are still waiting for registration.
- **Gentoo:** the `::snakebyte` overlay, maintained outside this project. See
  [docs/INSTALL-GENTOO.md](docs/INSTALL-GENTOO.md).
- **Build it yourself, any distribution:** [docs/INSTALL.md](docs/INSTALL.md).
- **The code without GitHub:** a mirror on our own site, kept in step with GitHub:
  `git clone https://ferrarivoz.com/git/AdobeWine.git`

Which distributions, graphics cards and desktops should work: [docs/COMPATIBILITY.md](docs/COMPATIBILITY.md).

## Why this exists

I'm not a Wine developer. I needed Adobe's apps for my work and didn't want to keep Windows just for them, so I
built this for my own laptop, with a lot of help from Claude, an AI coding assistant. I'm sharing it because many
people want to leave Windows or macOS but keep working with Adobe. Your reports, fixes and criticism are what will
make it work for everyone else.

## Help and feedback

- **Tried it?** Tell us your distribution, graphics card and what happened in a
  [compatibility report](../../issues/new?template=compatibility.yml), or use the Report a problem button.
- **Questions and ideas:** [Discussions](../../discussions). Where this is going: [docs/ROADMAP.md](docs/ROADMAP.md).
- **Want to help build it?** See [CONTRIBUTING.md](CONTRIBUTING.md). Patches are especially welcome for AMD and
  Intel graphics.

## How it works (for the curious)

AdobeWine is [Wine](https://www.winehq.org) 11.20, the compatibility layer that also runs Windows games on Steam,
plus 101 fixes for what Adobe's apps need. Each fix comes from comparing the same task on Windows and under Wine,
with a small test program that shows the difference. Every fix is listed in [docs/PATCHES.md](docs/PATCHES.md), the
method is in [docs/HOW-IT-WORKS.md](docs/HOW-IT-WORKS.md), and the code is public under Wine's own license. WineHQ doesn't accept AI-written code, so these fixes aren't
submitted there. Which Windows functions Adobe's apps call, and what happens with each one, is
mapped in [docs/DLL-MAP.md](docs/DLL-MAP.md). What changed in each version: [CHANGELOG.md](CHANGELOG.md).

Thanks to [PhialsBasement](https://github.com/PhialsBasement), whose Adobe installer work for Wine started the
first patches, and to NickPittas, switch87, nvb-uy, Cikappa2904 and everyone who has sent fixes, tests and reports.

## License

The patches and scripts are licensed under the GNU LGPL 2.1 or later, the same as Wine (see [LICENSE](LICENSE)).
DXVK, vkd3d-proton, nvidia-libs, wine-gecko and wine-mono come from their own projects and keep their own licenses.
