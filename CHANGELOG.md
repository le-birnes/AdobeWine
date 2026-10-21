# Changelog

## Unreleased

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
