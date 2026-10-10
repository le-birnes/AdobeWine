/* Smoke test for the XDG desktop portal file dialogs (patches 0060a/0060b).
 *
 *   portal.exe legacy   GetOpenFileNameW
 *   portal.exe save     GetSaveFileNameW
 *   portal.exe item     IFileOpenDialog with After Effects-like custom controls: an "Import As"
 *                       combo box in a visual group (item 2 selected), a "Sequence" check button
 *                       and a check button with an empty label
 *
 * Run with WINE_FORCE_PORTAL=1 (or HKCU\Software\Wine\X11 Driver FileDialogPortal=always) and a
 * session bus with org.freedesktop.portal.Desktop. Prints what the dialog returned, the custom
 * control states and the events seen; the last line is "RESULT ok|cancel|fail <detail>". */
#define COBJMACROS
#include <windows.h>
#include <commdlg.h>
#include <shobjidl.h>
#include <stdio.h>

#define ID_GROUP     100
#define ID_IMPORTAS  101
#define ID_SEQUENCE  102
#define ID_NOLABEL   103
#define ID_FMTTEXT   104
#define ID_FORMAT    105

static int file_ok_calls;

/* IFileDialogEvents + IFileDialogControlEvents, logging only */
struct events
{
    IFileDialogEvents IFileDialogEvents_iface;
    IFileDialogControlEvents IFileDialogControlEvents_iface;
};

static HRESULT WINAPI events_QueryInterface(IFileDialogEvents *iface, REFIID riid, void **out)
{
    struct events *This = CONTAINING_RECORD(iface, struct events, IFileDialogEvents_iface);

    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IFileDialogEvents))
        *out = &This->IFileDialogEvents_iface;
    else if (IsEqualIID(riid, &IID_IFileDialogControlEvents))
        *out = &This->IFileDialogControlEvents_iface;
    else
    {
        *out = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}
static ULONG WINAPI events_AddRef(IFileDialogEvents *iface) { return 2; }
static ULONG WINAPI events_Release(IFileDialogEvents *iface) { return 1; }

static void print_controls(IFileDialog *dlg, const char *when)
{
    IFileDialogCustomize *custom;
    DWORD item = 0;
    BOOL seq = -1, nolabel = -1;
    HRESULT hr;

    if (FAILED(IFileDialog_QueryInterface(dlg, &IID_IFileDialogCustomize, (void **)&custom))) return;
    hr = IFileDialogCustomize_GetSelectedControlItem(custom, ID_IMPORTAS, &item);
    IFileDialogCustomize_GetCheckButtonState(custom, ID_SEQUENCE, &seq);
    IFileDialogCustomize_GetCheckButtonState(custom, ID_NOLABEL, &nolabel);
    printf("%s: import-as item %lu (hr %#lx), sequence %d, no-label %d\n", when, item, hr, seq, nolabel);
    IFileDialogCustomize_Release(custom);
}

static HRESULT WINAPI events_OnFileOk(IFileDialogEvents *iface, IFileDialog *dlg)
{
    IShellItem *item;
    WCHAR *path;

    file_ok_calls++;
    if (SUCCEEDED(IFileDialog_GetResult(dlg, &item)))
    {
        if (SUCCEEDED(IShellItem_GetDisplayName(item, SIGDN_FILESYSPATH, &path)))
        {
            printf("OnFileOk: %ls\n", path);
            CoTaskMemFree(path);
        }
        IShellItem_Release(item);
    }
    else printf("OnFileOk: GetResult failed\n");
    print_controls(dlg, "OnFileOk");
    return S_OK;
}
static HRESULT WINAPI events_OnFolderChanging(IFileDialogEvents *iface, IFileDialog *dlg, IShellItem *item) { return S_OK; }
static HRESULT WINAPI events_OnFolderChange(IFileDialogEvents *iface, IFileDialog *dlg) { return S_OK; }
static HRESULT WINAPI events_OnSelectionChange(IFileDialogEvents *iface, IFileDialog *dlg) { return S_OK; }
static HRESULT WINAPI events_OnShareViolation(IFileDialogEvents *iface, IFileDialog *dlg, IShellItem *item,
                                              FDE_SHAREVIOLATION_RESPONSE *resp) { return S_OK; }
static HRESULT WINAPI events_OnTypeChange(IFileDialogEvents *iface, IFileDialog *dlg) { return S_OK; }
static HRESULT WINAPI events_OnOverwrite(IFileDialogEvents *iface, IFileDialog *dlg, IShellItem *item,
                                         FDE_OVERWRITE_RESPONSE *resp) { return S_OK; }

static IFileDialogEventsVtbl events_vtbl =
{
    events_QueryInterface, events_AddRef, events_Release, events_OnFileOk, events_OnFolderChanging,
    events_OnFolderChange, events_OnSelectionChange, events_OnShareViolation, events_OnTypeChange,
    events_OnOverwrite,
};

static struct events *impl_from_ctrl(IFileDialogControlEvents *iface)
{
    return CONTAINING_RECORD(iface, struct events, IFileDialogControlEvents_iface);
}
static HRESULT WINAPI cevents_QueryInterface(IFileDialogControlEvents *iface, REFIID riid, void **out)
{
    return events_QueryInterface(&impl_from_ctrl(iface)->IFileDialogEvents_iface, riid, out);
}
static ULONG WINAPI cevents_AddRef(IFileDialogControlEvents *iface) { return 2; }
static ULONG WINAPI cevents_Release(IFileDialogControlEvents *iface) { return 1; }
static HRESULT WINAPI cevents_OnItemSelected(IFileDialogControlEvents *iface, IFileDialogCustomize *c, DWORD ctl, DWORD item)
{
    printf("OnItemSelected: control %lu item %lu\n", ctl, item);
    return S_OK;
}
static HRESULT WINAPI cevents_OnButtonClicked(IFileDialogControlEvents *iface, IFileDialogCustomize *c, DWORD ctl) { return S_OK; }
static HRESULT WINAPI cevents_OnCheckButtonToggled(IFileDialogControlEvents *iface, IFileDialogCustomize *c, DWORD ctl, BOOL checked)
{
    printf("OnCheckButtonToggled: control %lu %d\n", ctl, checked);
    return S_OK;
}
static HRESULT WINAPI cevents_OnControlActivating(IFileDialogControlEvents *iface, IFileDialogCustomize *c, DWORD ctl) { return S_OK; }

static IFileDialogControlEventsVtbl cevents_vtbl =
{
    cevents_QueryInterface, cevents_AddRef, cevents_Release, cevents_OnItemSelected,
    cevents_OnButtonClicked, cevents_OnCheckButtonToggled, cevents_OnControlActivating,
};

static int test_item(void)
{
    static const COMDLG_FILTERSPEC filters[] = {{L"Images", L"*.png;*.exr"}, {L"All files", L"*.*"}};
    struct events events = {{&events_vtbl}, {&cevents_vtbl}};
    IFileDialogCustomize *custom;
    IFileDialog *dlg;
    IShellItem *item;
    DWORD cookie;
    WCHAR *path;
    HRESULT hr;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, &IID_IFileDialog, (void **)&dlg);
    if (FAILED(hr)) { printf("RESULT fail CoCreateInstance %#lx\n", hr); return 1; }

    IFileDialog_SetTitle(dlg, L"Import File (portal smoke test)");
    IFileDialog_SetFileTypes(dlg, ARRAYSIZE(filters), filters);
    IFileDialog_QueryInterface(dlg, &IID_IFileDialogCustomize, (void **)&custom);
    IFileDialogCustomize_StartVisualGroup(custom, ID_GROUP, L"Import As:");
    IFileDialogCustomize_AddComboBox(custom, ID_IMPORTAS);
    IFileDialogCustomize_AddControlItem(custom, ID_IMPORTAS, 1, L"Footage");
    IFileDialogCustomize_AddControlItem(custom, ID_IMPORTAS, 2, L"Composition");
    IFileDialogCustomize_AddControlItem(custom, ID_IMPORTAS, 3, L"Composition - Retain Layer Sizes");
    IFileDialogCustomize_SetSelectedControlItem(custom, ID_IMPORTAS, 2);
    IFileDialogCustomize_EndVisualGroup(custom);
    IFileDialogCustomize_AddCheckButton(custom, ID_SEQUENCE, L"ImporterJPEG Sequence", FALSE);
    IFileDialogCustomize_AddCheckButton(custom, ID_NOLABEL, L"", TRUE);
    /* After Effects labels a combo box with a text control in front of it, not a visual group */
    IFileDialogCustomize_AddText(custom, ID_FMTTEXT, L"Format:");
    IFileDialogCustomize_AddComboBox(custom, ID_FORMAT);
    IFileDialogCustomize_AddControlItem(custom, ID_FORMAT, 1, L"PNG");
    IFileDialogCustomize_AddControlItem(custom, ID_FORMAT, 2, L"TIFF");
    IFileDialogCustomize_SetSelectedControlItem(custom, ID_FORMAT, 1);
    IFileDialogCustomize_Release(custom);
    IFileDialog_Advise(dlg, &events.IFileDialogEvents_iface, &cookie);

    print_controls(dlg, "before");
    hr = IFileDialog_Show(dlg, NULL);
    printf("Show: %#lx, OnFileOk calls %d\n", hr, file_ok_calls);
    print_controls(dlg, "after");
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) { printf("RESULT cancel\n"); return 0; }
    if (FAILED(hr)) { printf("RESULT fail Show %#lx\n", hr); return 1; }
    if (FAILED(hr = IFileDialog_GetResult(dlg, &item))) { printf("RESULT fail GetResult %#lx\n", hr); return 1; }
    IShellItem_GetDisplayName(item, SIGDN_FILESYSPATH, &path);
    printf("RESULT ok %ls\n", path);
    CoTaskMemFree(path);
    IShellItem_Release(item);
    IFileDialog_Unadvise(dlg, cookie);
    IFileDialog_Release(dlg);
    return 0;
}

static int test_legacy(BOOL save)
{
    WCHAR file[MAX_PATH] = L"";
    OPENFILENAMEW ofn = {sizeof(ofn)};
    BOOL ret;

    if (save) lstrcpyW(file, L"untitled.aep");
    ofn.lpstrFilter = L"Projects (*.aep)\0*.aep\0All files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = file;
    ofn.nMaxFile = ARRAYSIZE(file);
    ofn.lpstrTitle = save ? L"Save As (portal smoke test)" : L"Open (portal smoke test)";
    ofn.Flags = OFN_EXPLORER | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    ret = save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
    printf("%s: %d, CommDlgExtendedError %#lx, offset %u, ext %u\n", save ? "GetSaveFileNameW" : "GetOpenFileNameW",
           ret, CommDlgExtendedError(), ofn.nFileOffset, ofn.nFileExtension);
    if (ret) printf("RESULT ok %ls\n", file);
    else if (!CommDlgExtendedError()) printf("RESULT cancel\n");
    else printf("RESULT fail %#lx\n", CommDlgExtendedError());
    return ret || !CommDlgExtendedError() ? 0 : 1;
}

int wmain(int argc, WCHAR **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc > 1 && !lstrcmpW(argv[1], L"item")) return test_item();
    if (argc > 1 && !lstrcmpW(argv[1], L"save")) return test_legacy(TRUE);
    return test_legacy(FALSE);
}
