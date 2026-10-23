/* Does ICreateTypeLib2::SaveAllChanges write a type library whose functions have empty doc strings?
 * InDesign's "Support for Visual Basic" plug-in builds its scripting type library (Resources for Visual Basic.tlb)
 * with CreateTypeLib2 and gives hundreds of functions SetFuncDocString(L""). Before patch 0088 Wine's save failed
 * with E_UNEXPECTED on the first empty string and wrote nothing, so InDesign's COM object could not resolve DoScript.
 *   emptydoc.exe       prints the HRESULTs of SaveAllChanges and LoadTypeLib, and the doc string read back
 * Build: x86_64-w64-mingw32-gcc -O2 -o emptydoc.exe emptydoc.c -loleaut32 -lole32 -luuid */
#define COBJMACROS
#include <windows.h>
#include <oleauto.h>
#include <stdio.h>

int main(void)
{
    static const GUID lib_guid = {0x1d6c2a7e, 0x5e0b, 0x4c39, {0x9a, 0x3e, 0x2b, 0x7f, 0x11, 0x6d, 0x52, 0x01}};
    static const GUID itf_guid = {0x1d6c2a7e, 0x5e0b, 0x4c39, {0x9a, 0x3e, 0x2b, 0x7f, 0x11, 0x6d, 0x52, 0x02}};
    WCHAR path[MAX_PATH], name[] = L"Run";
    LPOLESTR names[] = {name};
    ICreateTypeLib2 *ctl;
    ICreateTypeInfo *cti;
    ITypeLib *tl;
    FUNCDESC fd = {0};
    BSTR doc = NULL;
    HRESULT hr;
    int failures = 0;

    GetTempPathW(MAX_PATH, path);
    lstrcatW(path, L"emptydoc.tlb");
    DeleteFileW(path);

    hr = CreateTypeLib2(SYS_WIN64, path, &ctl);
    if (FAILED(hr)) { printf("CreateTypeLib2 0x%08lx\n", hr); return 1; }
    ICreateTypeLib2_SetGuid(ctl, &lib_guid);
    ICreateTypeLib2_SetName(ctl, (LPOLESTR)L"EmptyDoc");
    ICreateTypeLib2_SetDocString(ctl, (LPOLESTR)L"");                   /* empty library doc string too */
    ICreateTypeLib2_CreateTypeInfo(ctl, (LPOLESTR)L"IEmptyDoc", TKIND_DISPATCH, &cti);
    ICreateTypeInfo_SetGuid(cti, &itf_guid);

    fd.memid = 1; fd.funckind = FUNC_DISPATCH; fd.invkind = INVOKE_FUNC; fd.callconv = CC_STDCALL;
    fd.elemdescFunc.tdesc.vt = VT_VOID;
    hr = ICreateTypeInfo_AddFuncDesc(cti, 0, &fd);
    ICreateTypeInfo_SetFuncAndParamNames(cti, 0, names, 1);
    hr = ICreateTypeInfo_SetFuncDocString(cti, 0, (LPOLESTR)L"");      /* what InDesign does */
    printf("SetFuncDocString(L\"\") 0x%08lx\n", hr);
    ICreateTypeInfo_LayOut(cti);
    ICreateTypeInfo_Release(cti);

    hr = ICreateTypeLib2_SaveAllChanges(ctl);
    printf("SaveAllChanges 0x%08lx%s\n", hr, SUCCEEDED(hr) ? "" : "  FAIL");
    failures += FAILED(hr);
    ICreateTypeLib2_Release(ctl);

    hr = LoadTypeLibEx(path, REGKIND_NONE, &tl);
    printf("LoadTypeLibEx 0x%08lx%s\n", hr, SUCCEEDED(hr) ? "" : "  FAIL");
    failures += FAILED(hr);
    if (SUCCEEDED(hr))
    {
        ITypeInfo *ti;
        if (SUCCEEDED(ITypeLib_GetTypeInfo(tl, 0, &ti)))
        {
            BSTR n = NULL;
            hr = ITypeInfo_GetDocumentation(ti, 1, &n, &doc, NULL, NULL);
            printf("GetDocumentation 0x%08lx name %ls doc %s\n", hr, n ? n : L"(null)",
                   doc ? (*doc ? "(text)" : "empty") : "NULL");
            SysFreeString(n); SysFreeString(doc);
            ITypeInfo_Release(ti);
        }
        ITypeLib_Release(tl);
    }
    DeleteFileW(path);
    printf("%s\n", failures ? "FAIL" : "PASS");
    return failures;
}
