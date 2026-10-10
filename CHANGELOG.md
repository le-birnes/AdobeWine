# Changelog

## Unreleased

- Patch 0039 revised: chunked keep-alive replies no longer drop their connection (Wine's
  winhttp:notification tests pass again), and WinHttpReadData no longer waits to fill the
  buffer when some data is already there. New test: tests/winhttp/readavail.c.

## 0.2.0 - 2026-10-10

- Wine 11.18 with 59 patches.
- Patch 0058: `DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL`, which Photoshop's GPU check asks for
  (switch87, #6).
- Setup makes Segoe UI from Selawik when no Windows fonts are given, so Photoshop's menu bar
  no longer hangs without them (switch87, #3).
- Setup reports Direct3D 12 feature level 12_0/12_1 on GPUs whose Vulkan driver offers only 11_x
  (e.g. Mesa on Gen9 Intel), so Photoshop finds the GPU and Camera Raw can edit (switch87, #4).
- `adobewine <app>` and the menu entries start the newest installed release (e.g. Photoshop
  2025), and menu entries get the app icons again (switch87, #5).
- Gentoo install guide for the `::snakebyte` overlay (switch87, #7).
- Patch 0043 fixed: a program that added a font itself and then listed fonts could lose a font
  family or hang (found by Wine's own gdi32 tests).
- Patch 0046 narrowed: Wine's winhttp notification tests pass again; Photoshop still closes
  cleanly.
- Creative Cloud web installer 2.14.0.82 no longer shows a blank white window: patches 0055-0057
  (mshtml ChildNode methods and `indeterminate`, jscript `Object.entries/values/assign`). Cause
  found by NickPittas (#2). The temporary launcher workaround for 0.1.0 runtimes is gone again,
  as the runtime now has the fix.
- Experimental, off by default: Windows file dialogs can use your desktop's own file picker
  (XDG portal) with `WINE_FORCE_PORTAL=1`; After Effects' import options show in it. Patches
  0060a/0060b, ported from NickPittas' AE4Linux (LGPL-2.1+) and Wine merge request 10060 by
  Alexander Wilms.

## 0.1.0 - first public testing release

- Wine 11.18 with 53 patches (3 from PhialsBasement's Adobe installer work, 50 new).
- Tested on CachyOS with an NVIDIA RTX 4070 Laptop GPU: Creative Cloud desktop 6.10,
  Photoshop 27.10, Illustrator 30.8, Premiere Pro 26.5, Audition 26.5 and Media Encoder 26.5
  (see docs/STATUS.md).
- `adobewine` command: setup, app launching with Linux file paths, clean shutdown,
  sign-in fix, desktop menu entries, opt-in scrubbed crash reports.
- AUR packages `adobewine` (build) and `adobewine-bin` (prebuilt).
