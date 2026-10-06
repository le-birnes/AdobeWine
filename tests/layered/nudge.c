/* nudge.exe <title substring> <mode>: mode r = RedrawWindow all children, s = resize by +1 px and back */
#include <windows.h>
#include <stdio.h>
#include <string.h>
static const char *needle; static HWND found;
static BOOL CALLBACK find(HWND h, LPARAM l) { char t[256]; GetWindowTextA(h, t, sizeof(t)); if (IsWindowVisible(h) && strstr(t, needle)) { found = h; return FALSE; } return TRUE; }
int main(int argc, char **argv)
{
    RECT r;
    needle = argv[1]; EnumWindows(find, 0);
    if (!found) { printf("not found\n"); return 1; }
    if (argv[2][0] == 'r') printf("redraw %d\n", RedrawWindow(found, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW));
    else
    {
        GetWindowRect(found, &r);
        SetWindowPos(found, 0, 0, 0, r.right - r.left + 1, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        Sleep(500);
        SetWindowPos(found, 0, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        printf("resized\n");
    }
    return 0;
}
