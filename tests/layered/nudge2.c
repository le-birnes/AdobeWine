/* nudge2.exe <mode>: on the Creative Cloud window: c = InvalidateRect(child), t = InvalidateRect(top only),
 * p = PostMessage(child, WM_PAINT), a = SetForegroundWindow + SetActiveWindow, s = send WM_SIZE to top */
#include <windows.h>
#include <stdio.h>
int main(int argc, char **argv)
{
    HWND top = FindWindowA("Chrome_WidgetWin_0", "Creative Cloud Desktop"), child;
    RECT r;
    if (!top) { printf("no top\n"); return 1; }
    child = FindWindowExA(top, NULL, "Chrome_RenderWidgetHostHWND", NULL);
    switch (argv[1][0])
    {
    case 'c': printf("child %p %d\n", child, InvalidateRect(child, NULL, FALSE)); break;
    case 't': printf("top %d\n", InvalidateRect(top, NULL, FALSE)); break;
    case 'p': printf("post %d\n", PostMessageA(child, WM_PAINT, 0, 0)); break;
    case 'a': printf("fg %d\n", SetForegroundWindow(top)); break;
    case 'h': printf("hide child %d\n", ShowWindow(child, SW_HIDE)); break;
    case 'S': printf("show child %d\n", ShowWindow(child, SW_SHOWNA)); break;
    case 's': GetClientRect(top, &r); printf("size %ld\n", (long)SendMessageA(top, WM_SIZE, SIZE_RESTORED, MAKELPARAM(r.right, r.bottom))); break;
    }
    return 0;
}
