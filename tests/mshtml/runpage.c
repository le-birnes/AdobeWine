/* Load an HTML file into an MSHTML document (IE11 mode via FEATURE_BROWSER_EMULATION),
 * pump messages for a while and print document.title, where the page leaves its results. */
#define COBJMACROS
#include <windows.h>
#include <mshtml.h>
#include <urlmon.h>
#include <stdio.h>
#include <dispex.h>
#include <initguid.h>
DEFINE_GUID(IID_IDispatchEx_, 0xa6ef9860,0xc720,0x11d0,0x93,0x37,0x00,0xa0,0xc9,0x0d,0xca,0xa9);
DEFINE_GUID(CLSID_HTMLDocument_, 0x25336920,0x03f9,0x11cf,0x8f,0xd0,0x00,0xaa,0x00,0x68,0x6f,0x13);

int main(int argc, char **argv)
{
    IHTMLDocument2 *doc;
    IPersistMoniker *pm;
    IMoniker *mon;
    WCHAR url[MAX_PATH];
    BSTR title = NULL;
    DWORD mode = 11001, start;
    HKEY key;
    MSG msg;
    HRESULT hr;

    RegCreateKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION", &key);
    RegSetValueExA(key, "runpage.exe", 0, REG_DWORD, (BYTE *)&mode, sizeof(mode));
    RegCloseKey(key);

    CoInitialize(NULL);
    hr = CoCreateInstance(&CLSID_HTMLDocument_, NULL, CLSCTX_INPROC_SERVER, &IID_IHTMLDocument2, (void **)&doc);
    if (FAILED(hr)) { printf("create %#lx\n", hr); return 1; }
    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, url, MAX_PATH);
    CreateURLMoniker(NULL, url, &mon);
    IHTMLDocument2_QueryInterface(doc, &IID_IPersistMoniker, (void **)&pm);
    hr = IPersistMoniker_Load(pm, FALSE, mon, NULL, 0);
    printf("load %#lx\n", hr);

    if (argc > 3)  /* inject window.<argv[3]> from native code, the way Adobe Desktop Service does */
    {
        IHTMLWindow2 *win = NULL;
        IDispatchEx *dex = NULL;
        DISPID id;
        BSTR name;
        VARIANT v;
        DISPPARAMS dp;
        DISPID putid = DISPID_PROPERTYPUT;
        DWORD t0 = GetTickCount();

        /* wait for readyState complete first */
        for (;;)
        {
            BSTR rs = NULL;
            while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
            IHTMLDocument2_get_readyState(doc, &rs);
            if (rs && !wcscmp(rs, L"complete")) break;
            if (GetTickCount() - t0 > 5000) break;
            Sleep(10);
        }
        IHTMLDocument2_get_parentWindow(doc, &win);
        IHTMLWindow2_QueryInterface(win, &IID_IDispatchEx_, (void **)&dex);
        name = SysAllocString(L"JSObject");
        hr = IDispatchEx_GetDispID(dex, name, fdexNameEnsure, &id);
        printf("GetDispID ensure %#lx id %ld\n", hr, id);
        V_VT(&v) = VT_DISPATCH; V_DISPATCH(&v) = (IDispatch *)doc;
        dp.rgvarg = &v; dp.cArgs = 1; dp.rgdispidNamedArgs = &putid; dp.cNamedArgs = 1;
        hr = IDispatchEx_InvokeEx(dex, id, 0, DISPATCH_PROPERTYPUT, &dp, NULL, NULL, NULL);
        printf("InvokeEx put %#lx\n", hr);
    }
    start = GetTickCount();
    while (GetTickCount() - start < (argc > 2 ? atoi(argv[2]) : 3000))
    {
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
        Sleep(10);
    }
    IHTMLDocument2_get_title(doc, &title);
    printf("%ls\n", title ? title : L"(no title)");
    return 0;
}
