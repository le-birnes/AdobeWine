/* Print the Windows version as RtlGetVersion and kernel32's file version report it. */
#include <windows.h>
#include <stdio.h>
typedef LONG (WINAPI *rgv)(OSVERSIONINFOEXW *);
int main(void)
{
    OSVERSIONINFOEXW v = { sizeof(v) };
    char path[MAX_PATH]; DWORD h, n; void *buf; VS_FIXEDFILEINFO *ffi; UINT len;
    ((rgv)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion"))(&v);
    printf("RtlGetVersion %lu.%lu.%lu\n", v.dwMajorVersion, v.dwMinorVersion, v.dwBuildNumber);
    GetSystemDirectoryA(path, MAX_PATH); strcat(path, "\\kernel32.dll");
    n = GetFileVersionInfoSizeA(path, &h); buf = malloc(n);
    GetFileVersionInfoA(path, 0, n, buf); VerQueryValueA(buf, "\\", (void **)&ffi, &len);
    printf("kernel32 file %u.%u.%u.%u\n", HIWORD(ffi->dwFileVersionMS), LOWORD(ffi->dwFileVersionMS),
           HIWORD(ffi->dwFileVersionLS), LOWORD(ffi->dwFileVersionLS));
    return 0;
}
