/* addfont.exe <file>: AddFontResourceExW(file, 0), broadcast WM_FONTCHANGE, then stay alive 8 s.
 * Run fontfind.exe <family> from another process meanwhile: on Windows a non-private
 * AddFontResource is visible to every process of the session. */
#include <windows.h>
#include <stdio.h>
int wmain(int argc, WCHAR **argv)
{
    int n = AddFontResourceExW(argv[1], 0, NULL);
    printf("AddFontResourceExW -> %d\n", n); fflush(stdout);
    SendMessageTimeoutW(HWND_BROADCAST, WM_FONTCHANGE, 0, 0, SMTO_ABORTIFHUNG, 2000, NULL);
    Sleep(8000);
    RemoveFontResourceExW(argv[1], 0, NULL);
    return 0;
}
