/* fontlive.exe <family> <font file>: this process enumerates <family>, then another process
 * (addfont.exe) adds <font file> with AddFontResourceExW(path, 0), and this process enumerates
 * again. On Windows the second enumeration finds it (confirmed on Windows). */
#include <windows.h>
#include <stdio.h>

static int found;
static int CALLBACK cb(const LOGFONTW *lf, const TEXTMETRICW *tm, DWORD type, LPARAM l) { found++; return 1; }

static int count_family(const WCHAR *family)
{
    LOGFONTW lf = {0};
    HDC dc = GetDC(0);
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpynW(lf.lfFaceName, family, LF_FACESIZE);
    found = 0;
    EnumFontFamiliesExW(dc, &lf, cb, 0, 0);
    ReleaseDC(0, dc);
    return found;
}

int wmain(int argc, WCHAR **argv)
{
    STARTUPINFOW si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    WCHAR cmd[1024];
    int before, after;

    setvbuf(stdout, NULL, _IONBF, 0);
    before = count_family(argv[1]);
    printf("before %d\n", before);
    swprintf(cmd, ARRAYSIZE(cmd), L"addfont.exe \"%s\"", argv[2]);
    if (!CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) { printf("no addfont.exe\n"); return 1; }
    Sleep(2500);
    after = count_family(argv[1]);
    WaitForSingleObject(pi.hProcess, INFINITE);
    printf("before %d, after another process added it %d: %s\n", before, after, !before && after ? "PASS" : "FAIL");
    return !(!before && after);
}
