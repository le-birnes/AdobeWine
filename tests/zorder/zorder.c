/* Two Wine processes, the window manager switches between them: does Wine's z-order follow?
 *   zorder.exe big     a large window "zorder big" (process A)
 *   zorder.exe probe   a smaller window "zorder probe" (process B) that prints, twice a second, which window
 *                      WindowFromPoint returns at its own centre and which window is the foreground.
 * Activate "zorder big", then "zorder probe" with the window manager (tools/kwin-activate.sh): after the
 * switch the probe must find itself. Before patch 0087 Wine kept the other process's window on top, and
 * apps that check WindowFromPoint (Illustrator's canvas) ignored clicks.
 * Build: x86_64-w64-mingw32-gcc -O2 -o zorder.exe zorder.c -mwindows -luser32 -lgdi32 */
#include <windows.h>
#include <stdio.h>
#include <string.h>

static LRESULT CALLBACK proc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProcA(h, m, w, l);
}

int main(int argc, char **argv)
{
    BOOL probe = argc > 1 && !strcmp(argv[1], "probe");
    WNDCLASSA wc = {0};
    HWND hwnd;
    MSG msg;
    wc.lpfnWndProc = proc; wc.hInstance = GetModuleHandleA(NULL); wc.lpszClassName = probe ? "zprobe" : "zbig";
    wc.hbrBackground = (HBRUSH)GetStockObject(probe ? WHITE_BRUSH : GRAY_BRUSH);
    RegisterClassA(&wc);
    hwnd = CreateWindowA(wc.lpszClassName, probe ? "zorder probe" : "zorder big", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                         probe ? 300 : 0, probe ? 200 : 0, probe ? 600 : 1600, probe ? 400 : 1000, 0, 0, wc.hInstance, 0);
    if (probe) SetTimer(hwnd, 1, 500, NULL);
    while (GetMessageA(&msg, 0, 0, 0))
    {
        if (msg.message == WM_TIMER)
        {
            RECT r; POINT pt; char at[64] = "", fg[64] = "";
            GetWindowRect(hwnd, &r); pt.x = (r.left + r.right) / 2; pt.y = (r.top + r.bottom) / 2;
            GetWindowTextA(GetAncestor(WindowFromPoint(pt), GA_ROOT), at, sizeof(at));
            GetWindowTextA(GetForegroundWindow(), fg, sizeof(fg));
            printf("at centre: %s | foreground: %s\n", at, fg); fflush(stdout);
        }
        DispatchMessageA(&msg);
    }
    return 0;
}
