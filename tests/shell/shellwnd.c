/* What de-elevation code finds: shell window, Progman, Shell_TrayWnd, their processes. */
#include <windows.h>
#include <stdio.h>
static void show(const char *what, HWND w)
{
    DWORD pid = 0; char cls[64] = "";
    if (w) { GetWindowThreadProcessId(w, &pid); GetClassNameA(w, cls, sizeof(cls)); }
    printf("%-16s %p pid %04lx class %s\n", what, w, pid, cls);
}
int main(void)
{
    show("GetShellWindow", GetShellWindow());
    show("Progman", FindWindowA("Progman", NULL));
    show("Shell_TrayWnd", FindWindowA("Shell_TrayWnd", NULL));
    show("GetDesktopWindow", GetDesktopWindow());
    return 0;
}
