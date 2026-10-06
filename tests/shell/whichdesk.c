/* Print this process's window station and desktop, and whether a shell window is visible. */
#include <windows.h>
#include <stdio.h>
int main(void)
{
    WCHAR ws[64] = L"?", dk[64] = L"?"; DWORD n;
    GetUserObjectInformationW(GetProcessWindowStation(), UOI_NAME, ws, sizeof(ws), &n);
    GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()), UOI_NAME, dk, sizeof(dk), &n);
    printf("winsta %ls desktop %ls shell %p\n", ws, dk, GetShellWindow());
    return 0;
}
