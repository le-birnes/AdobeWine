#include <windows.h>
#include <stdio.h>
/* fontfind.exe <prefix>: list font families whose name starts with prefix (wide API, UTF-8 out) */
static const WCHAR *pre;
static int CALLBACK cb(const LOGFONTW *lf, const TEXTMETRICW *tm, DWORD t, LPARAM l)
{
    char s[256];
    if (_wcsnicmp(lf->lfFaceName, pre, wcslen(pre))) return 1;
    WideCharToMultiByte(CP_UTF8, 0, lf->lfFaceName, -1, s, sizeof(s), 0, 0); printf("%s\n", s); return 1;
}
int wmain(int c, WCHAR **v){ LOGFONTW lf = {0}; HDC dc = GetDC(0); pre = v[1]; lf.lfCharSet = DEFAULT_CHARSET; EnumFontFamiliesExW(dc, &lf, cb, 0, 0); return 0; }
