/* Enumerate DXCore adapters like Adobe Sensei does and create a D3D12 device on each.
 * Prints the HRESULTs and the adapter LUID / description. */
#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <stdio.h>
#include <d3d12.h>
#include <dxgi1_4.h>

DEFINE_GUID(IID_IDXCoreAdapterFactory, 0x78ee5945, 0xc36e, 0x4b13, 0xa6, 0x69, 0x00, 0x5d, 0xd1, 0x1c, 0x0f, 0x06);
DEFINE_GUID(IID_IDXCoreAdapterList, 0x526c7776, 0x40e9, 0x459b, 0xb7, 0x11, 0xf3, 0x2a, 0xd7, 0x6d, 0xfc, 0x28);
DEFINE_GUID(IID_IDXCoreAdapter, 0xf0db4c7f, 0xfe5a, 0x42a2, 0xbd, 0x62, 0xf2, 0xa6, 0xcf, 0x6f, 0xc8, 0x3e);
DEFINE_GUID(DXCORE_ADAPTER_ATTRIBUTE_D3D12_GRAPHICS, 0x0c9ece4d, 0x2f6e, 0x4f01, 0x8c, 0x96, 0xe8, 0x9e, 0x33, 0x1b, 0x47, 0xb1);
DEFINE_GUID(DXCORE_ADAPTER_ATTRIBUTE_D3D12_CORE_COMPUTE, 0x248e2800, 0xa793, 0x4724, 0xab, 0xaa, 0x23, 0xa6, 0xde, 0x1b, 0xe0, 0x90);

typedef struct IDXCoreAdapterFactory IDXCoreAdapterFactory;
typedef struct IDXCoreAdapterList IDXCoreAdapterList;
typedef struct IDXCoreAdapter IDXCoreAdapter;
struct IDXCoreAdapterFactoryVtbl { void *qi, *addref, *release;
    HRESULT (WINAPI *CreateAdapterList)(IDXCoreAdapterFactory *, UINT32, const GUID *, REFIID, void **); };
struct IDXCoreAdapterFactory { struct IDXCoreAdapterFactoryVtbl *lpVtbl; };
struct IDXCoreAdapterListVtbl { void *qi, *addref, *release;
    HRESULT (WINAPI *GetAdapter)(IDXCoreAdapterList *, UINT32, REFIID, void **);
    UINT32 (WINAPI *GetAdapterCount)(IDXCoreAdapterList *); };
struct IDXCoreAdapterList { struct IDXCoreAdapterListVtbl *lpVtbl; };
struct IDXCoreAdapterVtbl { HRESULT (WINAPI *qi)(IDXCoreAdapter *, REFIID, void **); void *addref, *release, *IsValid, *IsAttributeSupported, *IsPropertySupported;

    HRESULT (WINAPI *GetProperty)(IDXCoreAdapter *, UINT, size_t, void *); };
struct IDXCoreAdapter { struct IDXCoreAdapterVtbl *lpVtbl; };

int main(void)
{
    HRESULT (WINAPI *create)(REFIID, void **);
    HMODULE m = LoadLibraryA("dxcore.dll");
    IDXCoreAdapterFactory *f; IDXCoreAdapterList *list; UINT32 i, n;
    const GUID *attrs[] = {&DXCORE_ADAPTER_ATTRIBUTE_D3D12_GRAPHICS, &DXCORE_ADAPTER_ATTRIBUTE_D3D12_CORE_COMPUTE};
    int a;

    create = (void *)GetProcAddress(m, "DXCoreCreateAdapterFactory");
    printf("factory %#lx\n", create(&IID_IDXCoreAdapterFactory, (void **)&f));
    for (a = 0; a < 2; a++)
    {
        printf("attr %d list %#lx\n", a, f->lpVtbl->CreateAdapterList(f, 1, attrs[a], &IID_IDXCoreAdapterList, (void **)&list));
        n = list->lpVtbl->GetAdapterCount(list);
        for (i = 0; i < n; i++)
        {
            IDXCoreAdapter *ad; ID3D12Device *dev = NULL; LUID luid = {0}; char desc[256] = {0}; IDXGIAdapter *dx = NULL;
            list->lpVtbl->GetAdapter(list, i, &IID_IDXCoreAdapter, (void **)&ad);
            { UINT64 mem = 0; BOOL hw = 0;
            printf("  GetProperty luid %#lx desc %#lx mem %#lx hw %#lx\n",
                   ad->lpVtbl->GetProperty(ad, 0 /* InstanceLuid */, sizeof(luid), &luid),
                   ad->lpVtbl->GetProperty(ad, 2 /* DriverDescription */, sizeof(desc), desc),
                   ad->lpVtbl->GetProperty(ad, 7 /* DedicatedAdapterMemory */, sizeof(mem), &mem),
                   ad->lpVtbl->GetProperty(ad, 11 /* IsHardware */, sizeof(hw), &hw));
            printf("  mem %llu MB hw %d\n", (unsigned long long)(mem >> 20), hw); }
            printf("  adapter %u '%s' luid %08lx:%08lx qi(IDXGIAdapter) %#lx\n", i, desc, luid.HighPart, luid.LowPart,
                   ad->lpVtbl->qi(ad, &IID_IDXGIAdapter, (void **)&dx));
            printf("  D3D12CreateDevice(dxcore adapter, 11_0) %#lx\n",
                   D3D12CreateDevice((IUnknown *)ad, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, (void **)&dev));
            printf("  D3D12CreateDevice(dxcore adapter, 1_0_CORE) %#lx\n",
                   D3D12CreateDevice((IUnknown *)ad, 0x1000 /* D3D_FEATURE_LEVEL_1_0_CORE */, &IID_ID3D12Device, (void **)&dev));
        }
    }
    return 0;
}
