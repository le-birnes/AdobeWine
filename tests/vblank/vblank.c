/* D3DKMTWaitForVerticalBlankEvent must succeed and return once per refresh (Chromium's
 * GPU vsync thread paces frames with it; it was a stub failing at once). */
#include <windows.h>
#include <stdio.h>
typedef UINT D3DKMT_HANDLE;
typedef struct { HDC hDc; D3DKMT_HANDLE hAdapter; LUID AdapterLuid; UINT VidPnSourceId; } OPENADAPTER;
typedef struct { D3DKMT_HANDLE hAdapter, hDevice; UINT VidPnSourceId; } WAITVBLANK;
int main(void)
{
    HMODULE gdi = GetModuleHandleA("gdi32.dll");
    LONG (WINAPI *open)(OPENADAPTER *) = (void *)GetProcAddress(gdi, "D3DKMTOpenAdapterFromHdc");
    LONG (WINAPI *wait)(WAITVBLANK *) = (void *)GetProcAddress(gdi, "D3DKMTWaitForVerticalBlankEvent");
    OPENADAPTER oa = {GetDC(0)}; WAITVBLANK w = {0};
    LARGE_INTEGER f, a, b; LONG st = -1; int i;
    DEVMODEA dm = {.dmSize = sizeof(dm)};
    if (!wait) { printf("no D3DKMTWaitForVerticalBlankEvent export: FAIL\n"); return 1; }
    printf("open %lx\n", open(&oa));
    w.hAdapter = oa.hAdapter; w.VidPnSourceId = oa.VidPnSourceId;
    EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dm);
    QueryPerformanceFrequency(&f); QueryPerformanceCounter(&a);
    for (i = 0; i < 60; i++) if ((st = wait(&w))) break;
    QueryPerformanceCounter(&b);
    printf("status %lx, %d waits, %.2f ms each (display %lu Hz -> %.2f ms): %s\n", st, i,
           (b.QuadPart - a.QuadPart) * 1000.0 / f.QuadPart / (i ? i : 1), dm.dmDisplayFrequency, 1000.0 / dm.dmDisplayFrequency,
           !st && i == 60 ? "PASS" : "FAIL");
    return st != 0;
}
