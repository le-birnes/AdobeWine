/* gapmap: functions from docs/DLL-MAP.md exist, behave, or fail with an error instead of aborting. Prints PASS/FAIL. */
#include <windows.h>
#include <stdio.h>

static int fails;

static void check(int ok, const char *what)
{
    printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) fails++;
}

static void *get(const char *dll, const char *name)
{
    HMODULE m = LoadLibraryA(dll);
    return m ? (void *)GetProcAddress(m, name) : NULL;
}

int main(void)
{
    static const struct { const char *dll, *name; } names[] =
    {
        {"advapi32", "RegOpenKeyTransactedW"}, {"advapi32", "RegDeleteKeyTransactedW"},
        {"kernel32", "CreateFileMappingNumaA"}, {"kernel32", "CreatePrivateNamespaceW"},
        {"kernel32", "UnregisterApplicationRecoveryCallback"}, {"kernel32", "IsEnclaveTypeSupported"},
        {"kernel32", "PackageFamilyNameFromId"}, {"user32", "SkipPointerFrameMessages"},
        {"powrprof", "PowerReadACValue"}, {"propsys", "VariantCompare"}, {"mfplat", "MFCreateMediaBufferWrapper"},
        {"wininet", "InternetGetCookieEx2"}, {"wkscli", "NetWkstaGetInfo"}, {"spoolss", "OpenPrinterW"},
        {"userenv", "DeriveAppContainerSidFromAppContainerName"}, {"crypt32", "CryptDecryptMessage"},
    };
    LONG (WINAPI *open_tx)(HKEY, const WCHAR *, DWORD, REGSAM, HKEY *, HANDLE, void *) =
        get("advapi32", "RegOpenKeyTransactedW");
    int (WINAPI *compare)(const VARIANT *, const VARIANT *) = get("propsys", "VariantCompare");
    HRESULT (WINAPI *wrap)(void *, DWORD, DWORD, void **) = get("mfplat", "MFCreateMediaBufferWrapper");
    BOOL (WINAPI *usb_init)(HANDLE, void **) = get("winusb", "WinUsb_Initialize");
    LONG (WINAPI *jet_init)(void *) = get("esent", "JetInit");
    HRESULT (WINAPI *ws_send)(void *, void *, void *, void *) = get("websocket", "WebSocketSend");
    HANDLE m = NULL, (WINAPI *map)(HANDLE, void *, DWORD, DWORD, DWORD, const char *, DWORD) =
        get("kernel32", "CreateFileMappingNumaA");
    HKEY key = NULL;
    VARIANT a, b;
    void *out = NULL;
    int i;

    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) check(get(names[i].dll, names[i].name) != NULL, names[i].name);

    check(open_tx && !open_tx(HKEY_LOCAL_MACHINE, L"Software", 0, KEY_READ, &key, NULL, NULL) && key, "RegOpenKeyTransactedW opens");
    if (map) m = map(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, NULL, 0);
    check(m != NULL, "CreateFileMappingNumaA maps");

    a.vt = VT_I4; a.lVal = 1; b.vt = VT_I4; b.lVal = 2;
    check(compare && compare(&a, &b) < 0 && compare(&b, &a) > 0 && compare(&a, &a) == 0, "VariantCompare orders");
    check(wrap && wrap(NULL, 0, 0, &out) == E_POINTER, "MFCreateMediaBufferWrapper checks its arguments");

    SetLastError(0);
    check(usb_init && !usb_init(NULL, &out) && GetLastError() == ERROR_NOT_SUPPORTED, "WinUsb_Initialize fails cleanly");
    check(jet_init && jet_init(NULL) == -1001, "JetInit fails cleanly");
    check(ws_send && ws_send(NULL, NULL, NULL, NULL) == E_NOTIMPL, "WebSocketSend fails cleanly");
    return fails;
}
