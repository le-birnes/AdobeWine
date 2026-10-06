/* hung.exe <hwnd hex>: is the window's thread answering messages? */
#include <windows.h>
#include <stdio.h>
int main(int argc, char **argv)
{
    HWND hwnd = (HWND)(ULONG_PTR)strtoull(argv[1], NULL, 16);
    DWORD_PTR res; DWORD tid = GetWindowThreadProcessId(hwnd, NULL);
    LRESULT ok = SendMessageTimeoutA(hwnd, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 3000, &res);
    printf("tid %04lx IsHungAppWindow %d SendMessageTimeout %s\n", tid, IsHungAppWindow(hwnd), ok ? "answered" : "timed out");
    return 0;
}
