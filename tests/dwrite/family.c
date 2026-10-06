/* Which "Segoe UI*" families do GDI and DirectWrite report, and with which weights. */
#define COBJMACROS
#include <windows.h>
#include <dwrite.h>
#include <stdio.h>
#include <initguid.h>
DEFINE_GUID(IID_IDWriteFactory_, 0xb859ee5a,0xd838,0x4b5b,0xa2,0xe8,0x1a,0xdc,0x7d,0x93,0xdb,0x48);

static int CALLBACK enum_proc(const LOGFONTW *lf, const TEXTMETRICW *tm, DWORD type, LPARAM lp)
{
    const ENUMLOGFONTEXW *e = (const ENUMLOGFONTEXW *)lf;
    if (!wcsncmp(lf->lfFaceName, L"Segoe UI", 8))
        printf("gdi: face=%ls full=%ls style=%ls weight=%ld\n", lf->lfFaceName, e->elfFullName, e->elfStyle, lf->lfWeight);
    return 1;
}

int main(int argc, char **argv)
{
    IDWriteFactory *f;
    IDWriteFontCollection *c;
    HDC hdc = GetDC(0);
    LOGFONTW lf = {0};
    UINT32 i, j, n;
    lf.lfCharSet = DEFAULT_CHARSET;
    EnumFontFamiliesExW(hdc, &lf, enum_proc, 0, 0);

    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, &IID_IDWriteFactory_, (IUnknown **)&f);
    IDWriteFactory_GetSystemFontCollection(f, &c, FALSE);
    n = IDWriteFontCollection_GetFontFamilyCount(c);
    for (i = 0; i < n; i++)
    {
        IDWriteFontFamily *fam; IDWriteLocalizedStrings *s; WCHAR name[100];
        IDWriteFontCollection_GetFontFamily(c, i, &fam);
        IDWriteFontFamily_GetFamilyNames(fam, &s);
        IDWriteLocalizedStrings_GetString(s, 0, name, 100);
        if (!wcsncmp(name, L"Segoe UI", 8))
        {
            printf("dwrite: %ls:", name);
            for (j = 0; j < IDWriteFontFamily_GetFontCount(fam); j++)
            {
                IDWriteFont *font; IDWriteFontFamily_GetFont(fam, j, &font);
                printf(" %d/%d", IDWriteFont_GetWeight(font), IDWriteFont_GetStyle(font));
            }
            printf("\n");
        }
    }
    for (i = 1; i < argc; i++)
    {
        WCHAR w[100]; BOOL exists = FALSE; UINT32 idx = ~0u;
        MultiByteToWideChar(CP_ACP, 0, argv[i], -1, w, 100);
        IDWriteFontCollection_FindFamilyName(c, w, &idx, &exists);
        printf("find %s: exists=%d index=%u\n", argv[i], exists, idx);
    }
    return 0;
}
