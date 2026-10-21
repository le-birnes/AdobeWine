# Patches

AdobeWine is upstream Wine 11.18 plus these patches, applied in order. Each one fixes a
difference between Wine and Windows found by running the same Adobe workflow on both. How each
problem was found, with measurements and the test programs (in `tests/`), is in
[DEVLOG.md](DEVLOG.md); the method is in [HOW-IT-WORKS.md](HOW-IT-WORKS.md). Patches marked AdobeWine are meant to go upstream
to WineHQ; help with that is very welcome.

| # | Wine component | What it fixes or adds | Origin |
|---|---|---|---|
| 0001 | libs | Embedded xml decl | PhialsBasement |
| 0002 | msvcrt | Unlinkframe null | PhialsBasement |
| 0003 | d2d1 | Geometry realization bitmap rt | PhialsBasement |
| 0004 | user32 | Stub SetWindowFeedbackSetting | AdobeWine |
| 0005 | mshtml | Honour FEATURE_BROWSER_EMULATION | AdobeWine |
| 0006 | mshtml | Implement queryCommandSupported Enabled | AdobeWine |
| 0007 | mshtml | Location.port empty for default port | AdobeWine |
| 0008 | appxdeploymentclient | AddPackageAsync reports completion | AdobeWine |
| 0009 | kernelbase | Child inherits SetDllDirectory | AdobeWine |
| 0010 | dwrite | Resolve system font paths via data dir | AdobeWine |
| 0011 | winex11.drv | Client surface present uses toplevel visual | AdobeWine |
| 0012 | mshtml | Add window.atob and btoa | AdobeWine |
| 0013 | jscript | String.split empty separator embedded nulls | AdobeWine |
| 0014 | dxcore | Attributes and IsIntegrated IsDetachable | AdobeWine |
| 0015 | ntdll | Do not hold FLS lock across thread exit callbacks | AdobeWine |
| 0016 | ntdll | Heap fail instead of crashing on unmapped pointer | AdobeWine |
| 0017 | dwrite | Implement IDWriteTextLayout HitTestPoint | AdobeWine |
| 0018 | ntdll, winecfg | Windows 11 as build 26100 24H2 | AdobeWine |
| 0019 | win32u | Re present offscreen client surfaces after window surface flush | AdobeWine |
| 0020 | win32u | Re copy offscreen vulkan surfaces after sparse presents | AdobeWine |
| 0021 | d2d1 | Fix centre of rotated arcs bone shaped buttons | AdobeWine |
| 0022 | winmm | Init devices before answering preferred device queries | AdobeWine |
| 0023 |  | Heap skip corrupted free blocks instead of faulting under lock | AdobeWine |
| 0024 | d2d1 | Implement layers masks and opacity via offscreen bitmap | AdobeWine |
| 0025 | include, winex11.drv | Keep offscreen client windows at swapchain size and stretch | AdobeWine |
| 0026 | dwrite | Let trailing whitespace hang when wrapping lines | AdobeWine |
| 0027 | ntdll | Heap survive app heap corruption rebuild free lists keep subheaps | AdobeWine |
| 0028 | winhttp | Long header lines and no credentials on cross host redirect | AdobeWine |
| 0029 | windows.storage | Stream references clones and content type | AdobeWine |
| 0030 | winex11.drv | Manage popups owned by layered windows | AdobeWine |
| 0031 | windows.ui | Toast notification manager | AdobeWine |
| 0032 | ntdll | Heap quarantine for listed processes | AdobeWine |
| 0033 | rtworkq | Shut queues down without holding queues_section | AdobeWine |
| 0034 | d2d1 | Layer bitmaps are premultiplied | AdobeWine |
| 0035 | mshtml | MutationObserver and local script load events | AdobeWine |
| 0036 | ntdll, wow64 | Ntdll TokenLinkedToken size query | AdobeWine |
| 0037 | kernelbase | Token process from service on user desktop | AdobeWine |
| 0038 | include, ntdll, server | Ntdll token integrity levels | AdobeWine |
| 0039 | winhttp | Return available data and drop raw content length when decompressing | AdobeWine |
| 0040 | win32u | Font cache family name | AdobeWine |
| 0041 | rtworkq | Serial queue lifetime | AdobeWine |
| 0042 | d2d1 | No join for straight continuation | AdobeWine |
| 0043 | win32u | Pick up fonts added by other processes | AdobeWine |
| 0044 | ntdll | Wow64 remote breakin | AdobeWine |
| 0045 | win32u | Dialogwin trace channel | AdobeWine |
| 0046 | winhttp | Queue async receive response | AdobeWine |
| 0047 | gdi32, include, win32u, wow64win | D3dkmt open adapter from hdc and vblank wait | AdobeWine |
| 0048 | gdiplus | Honour padded source stride when blending PARGB to a DC | AdobeWine |
| 0049 | gdi32 | CreateBitmapIndirect reads rows bmWidthBytes apart | AdobeWine |
| 0050 | dxgi | Composition swapchains present to the DirectComposition target | AdobeWine |
| 0051 | dcomp | Minimal device target and visual for swapchain content | AdobeWine |
| 0052 | dwrite | Last resort font uses the system collection for ranges without one | AdobeWine |
| 0053 | win32u, winex11.drv | Recreated swapchains get the window size not the old offscreen size | AdobeWine |
| 0054 | ntdll | Private assemblies from the `<probing privatePath>` of a module's `.config` file (After Effects loads Mocha AE) | AdobeWine |
| 0055 | mshtml | ChildNode remove, replaceWith, before, after (cause found by NickPittas, #2) | AdobeWine |
| 0056 | mshtml | HTMLInputElement indeterminate is stored instead of E_NOTIMPL (NickPittas, #2) | AdobeWine |
| 0057 | jscript | Object.entries, Object.values, Object.assign in ES5+ modes (NickPittas, #2) | AdobeWine |
| 0058 | win32u | Implement DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL | switch87 |
| 0060a | comdlg32 | Experimental: XDG desktop portal file dialogs (your desktop's own file picker), off unless `WINE_FORCE_PORTAL=1` or `HKCU\Software\Wine\X11 Driver\FileDialogPortal` = `always`/`auto` | NickPittas (AE4Linux); Wine MR 10060 by Alexander Wilms |
| 0060b | comdlg32 | Portal dialogs: custom controls as choices (After Effects Import As / Sequence), document portal host paths | NickPittas (AE4Linux) |
| 0061 | d2d1 | Insert triangulation segments iteratively and give up when they do not converge (After Effects' Render Queue panel overflowed the stack) | AdobeWine |
| 0062 | server | Read and write process memory through a live thread id (winedbg on processes whose main thread exited) | AdobeWine |
| 0063 | dwrite | No FreeType auto-hinting, unhinted glyph run outlines as on Windows | nvb-uy |
| 0064 | dwrite | Quarter-pixel glyph positions in natural modes instead of truncated ones (uneven letter spacing) | nvb-uy |
| 0065 | win32u | No FreeType auto-hinting for GDI text | nvb-uy |
| 0066 | d2d1 | Draw to a multisampled copy of the target bitmap (`WINE_D2D_MSAA`, default 8, 0 off) | nvb-uy |
| 0067 | d2d1 | Analytic coverage for curve fills and strokes | nvb-uy |
| 0068a | include | SmoothingModeAntiAlias8x4 and SmoothingModeAntiAlias8x8 | Wine MR 12221 by Thomas Ritter |
| 0068b | gdiplus | Antialiased filled paths (Photoshop toolbar pills and icons) | Wine MR 12221 by Thomas Ritter |
| 0068c | gdiplus | Antialiased thin lines one pixel wide | Wine MR 12221 by Thomas Ritter |
| 0068d | gdiplus | Antialiased lines widened with the default flatness | Wine MR 12221 by Thomas Ritter |
| 0069 | win32u | Offscreen swapchains present immediately and vsync is paced to the display refresh rate, 59 to 237 fps at 240 Hz under XWayland (`WINE_OFFSCREEN_PACING`, `WINE_OFFSCREEN_REFRESH`) | nvb-uy |
| 0070 | win32u | Copy waited-for offscreen presents only after they completed | nvb-uy |
| 0071 | msvcp140 | Concurrency::details::platform::GetNextAsyncId (AdobeNotificationHelper aborted on the stub) | nvb-uy |
| 0072 | win32u | An explorer without a desktop window doesn't start another one (Creative Cloud's 6.11 self-update started 250 explorers and took the session down) | AdobeWine |
| 0073 | win32u | get_desktop_window connects a thread that never loaded user32 to the default desktop (Creative Cloud's update helper) | AdobeWine |
| 0074 | win32u | Processes with a restricted token read the display configuration (they saw a 1024x768 screen) | AdobeWine |
| 0075 | winex11, win32u, server | Pen tablets under XWayland from XInput2: continuous wintab packets with pressure, and Windows Ink pen pointers with pressure and tilt | AdobeWine |
| 0076 | d2d1 | Command lists play back in DrawImage() (After Effects' "Direct2D Drawbot error", blank panels) | AdobeWine |
| 0077 | win32u | The user lock and the hardware profile key are set up on first use, for processes that reach win32u before user32 initializes it (CC 6.11 update helpers hung on their first window) | AdobeWine |
| 0078 | win32u | Loading the font cache reuses a family the process already has; a duplicate (empty key left by Adobe Fonts sync, same name as a host font) corrupted the family tree and crashed every new process | AdobeWine |
| 0079 | d2d1 | Unpremultiply effect; DrawImage() draws Premultiply/Unpremultiply effects as their input (After Effects viewer overlay) | AdobeWine |
| 0080 | win32u | Imported Vulkan memory keeps its NT handle; no NULL Win32 handle for memory that was not exported | AdobeWine |
| 0081 | win32u | D3DKMT escapes returning the host fd of shared resources and fences (CUDA interop through nvcuda) | AdobeWine |
| 0082 | win32u | A window's offscreen present leaves out visible child windows that present their own surface (Premiere Pro's Import and Export modes stayed black under the main window's swapchain) | AdobeWine |
| 0083 | win32u | Re-presenting offscreen surfaces (0019) no longer waits for surfaces_lock under the user lock; Premiere deadlocked while dragging timeline clips | Cikappa2904 |
| 0084 | d2d1 | Shape shaders compiled once per process instead of per render target; Premiere's UI thread spent 71% of its time in d3dcompiler_47 | Cikappa2904 |
| 0085 | opencl | clCreateCommandQueueWithProperties (OpenCL 2.0), mapped to the 1.2 call; Premiere quit after the splash on AMD (rusticl) without it (#15) | AdobeWine |
| 0086 | rpcss | Classes registered by a COM server that was killed or crashed are dropped, so scripts reach the restarted app (0x800706ba before) | AdobeWine |
