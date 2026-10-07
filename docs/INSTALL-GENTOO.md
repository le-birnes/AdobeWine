# Installing AdobeWine on Gentoo

Gentoo packages for AdobeWine are in the `::snakebyte` overlay
(<https://github.com/switch87/snakebyte-overlay>), maintained outside this project by
Gert Pellin. Portage builds the patched Wine and installs DXVK, vkd3d-proton, wine-gecko
and wine-mono itself, so `adobewine setup` downloads nothing but what winetricks fetches.

| Package | What it does |
|---|---|
| `app-emulation/wine-adobe` | Wine 11.18 with the AdobeWine patches, in `/usr/lib/wine-adobe-11.18` (slotted next to other Wine versions) |
| `app-emulation/adobewine` | the `adobewine` command and scripts in `/opt/adobewine`, DXVK, vkd3d-proton, the D3D12 shim |
| `media-fonts/selawik` | Microsoft's open Segoe UI fallback, used when you have no Windows fonts |

The overlay may carry fixes that are not in an AdobeWine release yet; its README lists
them.

## 1. Add the overlay

With `app-eselect/eselect-repository`:

```sh
eselect repository add snakebyte git https://github.com/switch87/snakebyte-overlay.git
emaint sync -r snakebyte
```

## 2. Install

The packages are keyworded `~amd64`:

```sh
cat >> /etc/portage/package.accept_keywords/adobewine <<'KW'
app-emulation/adobewine ~amd64
app-emulation/wine-adobe ~amd64
media-fonts/selawik ~amd64
KW
emerge -av app-emulation/adobewine
```

`wine-adobe` builds Wine (20-60 minutes) and needs its default USE flags `gecko`, `mono`,
`vulkan` and `wow64`; Portage says so if your configuration turns one of them off. A
working Vulkan driver is required (`media-libs/mesa` with `VIDEO_CARDS` for your GPU, or
`x11-drivers/nvidia-drivers`).

## 3. Use it

The same as on other distributions, from [INSTALL.md](INSTALL.md) section 3 on:

```sh
adobewine setup --windows-fonts /path/to/Windows/Fonts  # or without, see below
adobewine run ~/Downloads/Creative_Cloud_Set-Up.exe       # install Creative Cloud, sign in
adobewine launchers                                       # add the apps to your menu
```

Without `--windows-fonts`, setup makes Segoe UI from Selawik. After an update of
`app-emulation/adobewine`, run `adobewine setup` once more so your prefix gets the
changes.
