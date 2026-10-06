# How AdobeWine works

## The pieces

| Layer | What it does | Project |
|---|---|---|
| Wine 11.18 + AdobeWine patches | Windows API on Linux | this repository |
| DXVK 3.1.1 | Direct3D 8-11 on Vulkan (UI, canvas, video effects) | doitsujin/dxvk |
| vkd3d-proton 3.0.1 | Direct3D 12 on Vulkan (Illustrator canvas, Photoshop AI features) | HansKristian-Work/vkd3d-proton |
| nvidia-libs 1.0.2 | CUDA, NVENC/NVDEC, NVAPI, NVML for Windows programs on the Linux NVIDIA driver | SveSop/nvidia-libs |
| wine-gecko, wine-mono | Internet Explorer engine (sign-in, installer pages), .NET | WineHQ |
| GStreamer | media decoding behind Wine's Media Foundation | your distribution |

The Adobe apps install themselves through Adobe's Creative Cloud app, exactly as on
Windows, and sign in with your Adobe account.

## The method: record Windows, compare, fix

Guessing why a large closed-source app misbehaves under Wine is slow. AdobeWine instead
compared the same workflow on a real Windows 11 machine and under Wine:

1. **Inventory**: every DLL and function each Adobe binary imports, matched against what
   Wine implements, gave the list of possible gaps.
2. **Recordings on Windows**: API call traces (Frida), file and registry activity
   (Process Monitor), window trees and timings for each scenario: launch, sign-in, open,
   edit, export.
3. **The same scenario under Wine**, with Wine's debug channels, then the first point
   where the two differ.
4. **A minimal test program** that reproduces the difference, run on both Windows and
   Wine. The fix makes Wine give the Windows answer; the test stays in `tests/`.
5. **Reference outputs**: exports made on Windows (PNG, PDF, WAV, H.264) are compared
   with the same exports under AdobeWine, pixel by pixel or sample by sample.

The [development log](DEVLOG.md) has every step, including dead ends.

## Typical kinds of fixes

- **Missing or incomplete APIs**: DirectComposition (needed by WebView2), DXGI
  composition swapchains, D3DKMT vertical-blank waits, `IDWriteTextLayout::HitTestPoint`.
- **Behaviour differences**: `CreateBitmapIndirect` honouring `bmWidthBytes`, WinHTTP
  asynchronous receive semantics, token integrity levels, font cache refresh across
  processes.
- **Robustness against the apps' own bugs**: Creative Cloud helpers write to freed heap
  memory; Windows tolerates it, so does AdobeWine (heap quarantine, patch 0032).
- **Rendering**: Direct2D layers, masks and arc geometry; offscreen Vulkan surfaces for
  child windows; swapchain resizing.
