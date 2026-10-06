/* parents.exe <hwnd hex>: print a window and its ancestors (class, pid, rect, visible). */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
    HWND h = (HWND)(ULONG_PTR)strtoull(argv[1], NULL, 16);
    while (h)
    {
        char cls[128]; RECT r; DWORD pid;
        GetClassNameA(h, cls, sizeof(cls)); GetWindowRect(h, &r); GetWindowThreadProcessId(h, &pid);
        printf("%p pid %04lx %-34s (%ld,%ld)-(%ld,%ld) %s style %08lx\n", h, pid, cls, r.left, r.top, r.right, r.bottom,
               IsWindowVisible(h) ? "vis" : "hid", GetWindowLongA(h, GWL_STYLE));
        h = GetAncestor(h, GA_PARENT);
        if (h == GetDesktopWindow()) break;
    }
    return 0;
}
