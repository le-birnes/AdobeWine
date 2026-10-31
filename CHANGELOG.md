# Changelog

## 0.5.0 - 2026-10-31

- **Five more apps in the menu and the AdobeWine window:** InDesign, Bridge, Lightroom Classic, Lightroom and Acrobat.
  `adobewine off` closes them too.
- **Acrobat no longer crashes at start** in the setups where it did: two bugs in Wine's signature checking, one a
  race between threads (0103). The background helpers acrotray and AdobeCollabSync stopped aborting too.
- **The Windows side is mapped:** every Windows function imported by 5664 Adobe program files was checked against
  Wine ([docs/DLL-MAP.md](docs/DLL-MAP.md)). Of 107 gaps, 104 are closed (0101, 0102). Most of those report "not
  available here" where Windows without that feature would; they don't add the feature. The other 3 belong to DXVK
  and vkd3d. A function that exists isn't proof it behaves like Windows: that is what the app tests are for.
- Arch: a ready-made package on the release page, installed with one `pacman -U` command.
- **Now based on Wine 11.20**, with 101 patches. New: patches 0092-0103 (installers, Acrobat, thread start-up).
- Bridge, Lightroom Classic and Lightroom install from Creative Cloud and open (first tests, 2026-10-30).
- Lightroom Classic: 21 everyday functions tested and working, including AI Subject masking. Bridge: 26 tested and working, including Batch Rename, Camera Raw, Image Processor and Output to PDF.
- A failed install no longer breaks the other apps: when Acrobat's installer failed, Wine's rollback deleted the
  Microsoft Visual C++ runtime that every Adobe app needs (patches 0093, 0094).
- **Acrobat Pro installs and works.** Wine can now apply the update patches Acrobat's installer uses (0097, written
  clean-room from a behaviour description and checked against Windows: all 3534 of Acrobat's patches come out
  identical), and installs the update's files from the right place (0096, 0098). 22 everyday functions tested and
  working: open and read, comments and highlights, adding text, rotating, form fields, combining, extracting and
  deleting pages, export to Word, Excel, PowerPoint, RTF, text, JPEG and PNG, signing with a digital ID, zoom and
  page layout, and creating a PDF from a JPEG, PNG, BMP, TIFF or GIF (0099: Acrobat converts images in a small
  sandbox and checks who owns its connection to it; Wine now answers as Windows does), printing (through Wine's
  PostScript driver), OCR of a scanned page, and Fill & Sign (text and check marks). Acrobat's Protected Mode is
  switched off (Wine has no sandbox for it).
- Bridge: 26 everyday functions tested and working (browse, previews, ratings, keywords, metadata, collections,
  stacks, batch rename, Camera Raw, Image Processor, the Export panel, contact sheets to PDF, Filter panel, search,
  workspaces).
- Lightroom Classic: 18 everyday functions tested and working (import, ratings, keywords, collections, Develop
  edits and presets, virtual copies, export to JPEG, TIFF, PSD and DNG). Exporting to a folder on a separate
  disk or partition (for example a separate /home) failed with "The file could not be written"; fixed (0095).

## 0.4.1 - 2026-10-25

- New README for people who want to try Linux and keep working with Adobe: install in five steps, what works, what
  to know; the technical parts moved down or into docs/.
- The Flatpak's first start is faster and smaller: from Open to Adobe's installer in about 50 s instead of 90 s,
  and about 210 MB less on disk per user. Setup unpacks the core fonts itself instead of through winetricks
  (2 s instead of 45 s, same files and checksums), hardlinks Windows Installer's copies of Gecko and Mono to the
  downloads instead of storing them twice, and drops the DXVK/vkd3d-proton/nvidia-libs archives once unpacked.
- The first-run window downloads Creative Cloud with curl: Adobe's server ended about half of the previous
  downloads at 1 MB, and three failures in a row stopped the setup with an error.
- Gecko and Mono downloads are checked against their checksums, so a download cut short is fetched again
  instead of kept.

## 0.4.0 - 2026-10-24

- **One file for every distribution: `CCINSTALLER.flatpak`.** Double-click it, click Install in your app store, then
  Open. A small window checks the computer (Vulkan GPU, disk space), sets everything up with a progress bar,
  downloads Adobe's current Creative Cloud installer from adobe.com and starts it, and stops at Adobe's sign-in.
  Afterwards it lists the installed apps with Open buttons, adds them to the app menu, closes them all, and makes
  problem reports (shown in full before anything is sent). After an AdobeWine update it refreshes the setup once.
  The Flatpak carries Wine with all patches, the patched GPU layers, the Windows-side helpers, Selawik as a Segoe UI
  stand-in, winetricks and the rest, on the GNOME runtime from Flathub. Recipe: `packaging/flatpak`, build with
  `packaging/flatpak/make-bundle.sh`; distribution check: `tests/distro/flatpak-check.sh`.
- Setup no longer stops on a brand-new prefix: the InDesign scripting check added for 0088 ended setup when
  InDesign had never been installed (new in 0.3.1), nor where `xrdb` is missing (the desktop's scale
  then cannot be read and 96 DPI is used).
- Setup downloads Gecko and Mono before Wine's first boot and puts them where Wine looks for them, so Wine installs
  and registers them silently instead of asking in a dialog. Adobe's current web installer (5.3.5.13) needs the
  browser engine registered from that first boot; without it the installer window stayed hidden.
- With `ADOBEWINE_LINK_DLLS=1` (set by the Flatpak), setup links Wine's own DLLs into the prefix instead of copying
  them, as Proton does: a new prefix is 773 MB instead of 2.4 GB.
- Shader caches (vkd3d-proton, DXVK) go to `~/.local/share/adobewine/cache/shaders` instead of the folder an app
  was started from.
- `adobewine launchers` writes menu entries that run `ADOBEWINE_EXEC` when it is set (the Flatpak's own command).

- Illustrator: floating panels (Transform, Color, Align...) show again; they were invisible and blocked clicks on
  the part of the canvas they covered (patch 0091).
- InDesign: after double-clicking or dragging over text, Ctrl shortcuts work again (Ctrl+Shift+B typed a "B" over
  the word, Ctrl+A typed "a"). InDesign activates a small invisible window over the selection, and Wine handed it to
  the window manager, which moved the keyboard focus around mid-shortcut (patch 0090).
- After Effects' Render Queue panel works by hand: opening it (Ctrl+Alt+0, or Composition > Add to Render Queue)
  left it blank and stopped After Effects' background work, so scripts timed out. A shape the panel draws failed
  to build in Wine's Direct2D; a group made from it then read freed memory, and the fault left After Effects half
  way through building the panel (patch 0089). Add to Render Queue, Output To and Render now work from the panel.
- InDesign can be scripted from other programs over COM again (VBScript, Python, automation tools): it builds its
  scripting type library with Wine's CreateTypeLib2 and gives hundreds of functions an empty doc string, which made
  Wine's save fail and write nothing, so InDesign answered "unknown name" for DoScript (patch 0088,
  `tests/typelib/emptydoc.c`). Rerun `adobewine setup` after updating: it removes the stale registration, and
  InDesign writes the library on its next start.

## 0.3.1 - 2026-10-23

- Showreel: a short film made entirely in the Adobe apps under AdobeWine, every file created,
  rendered and exported on Linux: https://youtu.be/3Vbzt4cGSQQ. Photoshop painting with pen
  pressure, Illustrator Blob Brush lettering and Pathfinder, an InDesign report (styles, table,
  placed PSD and AI files, PDF export), Audition music (Generate Tones, Studio Reverb, Normalize),
  After Effects 3D titles (Cinema 4D renderer), shape layers, particles and audio spectrum rendered
  with aerender, and the cut, Lumetri grades and transitions in Premiere Pro, exported through
  Media Encoder. The pen strokes come from the simulated tablet (`tools/pen.py`) and most steps were
  scripted.
- `tools/pen.py`: `curve` (a quadratic stroke) and `path` (one stroke through many points, pressure
  swelling from `PEN_LO` to `PEN_HI` and back) for drawing lettering and brush strokes.
- Illustrator's canvas works with mouse and pen while another Adobe app is open: switching apps left the
  other app's window on top in Wine's own window order, so Illustrator ignored every click and drag on
  the canvas (patch 0087, `tests/zorder/zorder.c`).
- Scripting an Adobe app over COM (as automation tools and other apps do) works again after the
  app was killed or crashed and restarted; Wine's rpcss kept the dead instance's registration
  (patch 0086).

## 0.3.0 - 2026-10-21

- Packages: the AUR packages build the patched vkd3d-proton and CUDA bridge the After Effects viewers
  need (`scripts/build-gpu-deps.sh`) and `adobewine setup` installs them; rerun `adobewine setup`
  after updating.
- Premiere Pro on AMD GPUs with Mesa's OpenCL (rusticl): it quit right after the splash
  because Wine's OpenCL lacked `clCreateCommandQueueWithProperties`, which Premiere's OpenVINO GPU
  plugin imports (patch 0085, `tests/opencl/cqprops.c`; confirmation on AMD pending). Found with
  @pausebreakalex's logs (#15); on older releases, `OCL_ICD_VENDORS=/nonexistent adobewine premiere`
  works around it.
- Premiere Pro no longer freezes for good while dragging clips in the timeline (a lock-order
  deadlock in the offscreen-surface re-copy, patch 0083), and its UI no longer spends most of its
  time recompiling Direct2D shaders for every render target (patch 0084, render targets ~25x faster
  to create). Both by @Cikappa2904 (#14), with `tests/layered/flushrace.c` and `tests/d2d/rtcreate.c`.
- Real production job as an end-to-end test: a 320-layer After Effects comp with animated captions
  renders like on Windows, and its Premiere cut, with the comp through Dynamic Link (a headless After
  Effects renders it), exports 10 s in 14.5-20 s (Windows 19.6 s); the output matches Windows' export of the same cut
  (SSIM 0.993, PSNR 49 dB, audio identical).
- Premiere Pro: the Import and Export modes draw; they stayed black. The main window has its own
  always-black Direct3D swapchain that was copied over the modes' child windows (patch 0082).
- After Effects: the Composition and Layer viewers show the composition again with GPU (CUDA) rendering on
  NVIDIA; they stayed black. AE passes CUDA frames to its Direct3D 12 viewer through CUDA interop: patches
  0079-0081, a vkd3d-proton patch (shared buffers exportable) and a CUDA bridge patch (D3D11 interop,
  D3D handles through Wine's D3DKMT), built by `scripts/build-gpu-deps.sh` and installed by setup.
- Pen tablets work under XWayland without extra software (patch 0075): Photoshop (set
  `UseSystemStylus 0` in `PSUserConfig.txt`), Illustrator and After Effects follow pen pressure, and
  InDesign and Premiere get the pen as a pen. Wine now reads the tablet through XInput2 and also
  delivers it as Windows Ink pen input. `tools/pen.py` simulates a pen for testing; `tests/pen` has
  the test scripts and a probe (#13).
- After Effects: no more "Direct2D Drawbot error" on every redraw; its panels draw again (patch 0076).
- Creative Cloud's self-update to 6.11 no longer takes the whole desktop session down: a helper
  started without a desktop made Wine start explorer after explorer (250 in a second) until X ran
  out of clients and memory and swap filled (patch 0072). New `tools/sandbox.sh` runs a command
  in a memory-capped systemd scope and kills it if it runs away, or when X connections near
  XWayland's limit.
- Creative Cloud's update helpers get a desktop (patch 0073) and, running with a restricted token,
  can read the display configuration again (patch 0074; before, they saw a 1024x768 screen).
  Helpers that reach Wine's window code before user32 sets it up no longer hang on their first
  window or lose the display configuration (patch 0077); the step that swaps the updated files in
  now finishes, and the 6.11 self-update completes on its own, including the Visual C++ runtime
  it installs (patch 0078).
- Creative Cloud's Adobe Fonts sync no longer makes every program started afterwards crash (patch
  0078): a font it added and removed again that has the same name as one of your system fonts (Open
  Sans) corrupted Wine's font list in each new process. This was behind the VC++ installer failing
  during the update and the storms of crashing console windows.
- Photoshop rendering and speed on Wayland/NVIDIA, by nvb-uy (#12): text without FreeType
  auto-hinting and with quarter-pixel glyph positions like on Windows, no colour fringes next to
  Direct2D text (`config/fonts.conf`), multisampled and analytic antialiasing in Direct2D,
  antialiased GDI+ paths and lines (Wine MR 12221 by Thomas Ritter), offscreen swapchains paced
  to the display refresh rate, and AdobeNotificationHelper no longer aborts (patches 0063-0071).
- Heap patch 0027 revised: emptied heap segments are still kept mapped for apps that touch freed
  memory, but are no longer reused, so allocations are placed as in stock Wine (Wine's kernel32 heap
  test had 4 failures on 32-bit). New test tests/heap/keep.c.
- After Effects loads the Mocha AE plug-in (patch 0054, private assemblies through a `.config`
  file's privatePath). It was written right after 0.1.0 but had been left out of the public
  patch set by mistake.

## 0.2.2 - 2026-10-11

- After Effects renders: adding a comp to the Render Queue no longer kills its main thread (stack
  overflow in Wine's Direct2D triangulation, patch 0061). Tested with a real 300-layer comp.
- winedbg can attach to processes whose main thread has exited (patch 0062).
- Desktop file picker (experimental, `WINE_FORCE_PORTAL=1`): After Effects' "Format:" list shows its real
  label instead of "Option 1" (patch 0060b). Portal tests added in tests/portal.
- HiDPI: `adobewine setup` sets Wine's DPI from your desktop's scale (Xft.dpi), so the Adobe apps are
  no longer tiny on 1.5x/2x screens; `--dpi N` chooses another value.
- Illustrator's PSD and SVG exports use Myriad Pro like on Windows (they fell back to Times New
  Roman): the launcher puts Adobe's core fonts (Myriad Pro, Minion Pro) from an installed app into
  Common Files\\Adobe\\Fonts, the shared folder Adobe's installers create on Windows.

## 0.2.1 - 2026-10-10

- `adobewine aftereffects` starts After Effects (newest installed release).
- The launcher notices an app that closed but did not exit (no window, process still there) and
  ends only that app, instead of handing the new launch to it so that nothing opens.
- `bin/env.sh` works when sourced from zsh.
- Patch 0039 revised: chunked keep-alive replies no longer drop their connection (Wine's
  winhttp:notification tests pass again), and WinHttpReadData no longer waits to fill the
  buffer when some data is already there. New test: tests/winhttp/readavail.c.
- Patch 0035 revised: local scripts' load event fires after their 'complete' state change, as in
  IE, so Wine's mshtml script tests no longer hang. New test page: tests/mshtml/loadevents.html.

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
