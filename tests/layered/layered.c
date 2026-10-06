/* Unpainted layered window with colour key black + alpha 3 (what Photoshop 2026 creates):
 * who receives a real click at its centre, the layered window or the window behind it?
 * Prints the pixel of the unpainted surface and the window that got WM_LBUTTONDOWN. */
#include <windows.h>
#include <stdio.h>

static HWND back, top, got;

static LRESULT CALLBACK proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_LBUTTONDOWN) { got = hwnd; return 0; }
    if (msg == WM_ERASEBKGND && hwnd == top) return 1;  /* never paints, like Photoshop's dialog */
    if (msg == WM_PAINT && hwnd == top) { PAINTSTRUCT ps; BeginPaint(hwnd, &ps); EndPaint(hwnd, &ps); return 0; }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static void pump(DWORD ms)
{
    DWORD end = GetTickCount() + ms; MSG m;
    while ((LONG)(end - GetTickCount()) > 0)
    {
        while (PeekMessageA(&m, 0, 0, 0, PM_REMOVE)) { TranslateMessage(&m); DispatchMessageA(&m); }
        Sleep(10);
    }
}

int main(int argc, char **argv)
{
    WNDCLASSA wc = {0};
    INPUT in[2] = {0};
    POINT old, c = {300, 300};
    HDC dc;
    int alpha = argc > 1 ? atoi(argv[1]) : 3;

    wc.lpfnWndProc = proc; wc.hInstance = GetModuleHandleA(0); wc.lpszClassName = "lwtest";
    wc.hbrBackground = GetStockObject(GRAY_BRUSH); wc.hCursor = LoadCursorA(0, (LPCSTR)IDC_ARROW);
    RegisterClassA(&wc);
    back = CreateWindowExA(WS_EX_TOPMOST, "lwtest", "back", WS_POPUP | WS_VISIBLE, 100, 100, 400, 400, 0, 0, 0, 0);
    top = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST, "lwtest", "top", WS_POPUP, 150, 150, 300, 300, back, 0, 0, 0);
    SetLayeredWindowAttributes(top, RGB(0, 0, 0), alpha, LWA_COLORKEY | LWA_ALPHA);
    ShowWindow(top, SW_SHOWNOACTIVATE);
    SetForegroundWindow(back);
    pump(700);

    dc = GetDC(top);
    printf("alpha %d unpainted surface pixel %06lx\n", alpha, GetPixel(dc, 150, 150));
    ReleaseDC(top, dc);
    printf("WindowFromPoint(centre) = %s\n", WindowFromPoint(c) == top ? "top (layered)" :
           WindowFromPoint(c) == back ? "back" : "other");

    GetCursorPos(&old);
    SetCursorPos(c.x, c.y);
    pump(200);
    in[0].type = in[1].type = INPUT_MOUSE;
    in[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN; in[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, in, sizeof(INPUT));
    pump(500);
    SetCursorPos(old.x, old.y);
    printf("click went to %s\n", got == top ? "top (layered)" : got == back ? "back" : "nobody/other");
    DestroyWindow(top); DestroyWindow(back);
    return 0;
}
