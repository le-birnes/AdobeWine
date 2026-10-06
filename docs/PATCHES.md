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
