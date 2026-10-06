/* msgwnds.exe: list message-only windows (class, title, pid, tid). */
#include <windows.h>
#include <stdio.h>
int main(void)
{
    HWND h = NULL;
    while ((h = FindWindowExA(HWND_MESSAGE, h, NULL, NULL)))
    {
        char cls[128] = "", t[128] = ""; DWORD pid, tid = GetWindowThreadProcessId(h, &pid);
        GetClassNameA(h, cls, sizeof(cls)); GetWindowTextA(h, t, sizeof(t));
        printf("%p pid %04lx tid %04lx %-40s %s\n", h, pid, tid, cls, t);
    }
    return 0;
}
