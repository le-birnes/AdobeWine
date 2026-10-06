/* d3d12.dll shim in front of vkd3d-proton (renamed d3d12_vkd3d.dll).
 * vkd3d-proton rejects D3D_FEATURE_LEVEL_1_0_CORE (compute-only devices, used by
 * DirectML / Adobe Sensei through DXCore). A feature level 11_0 device offers everything a
 * core device does, so such requests are retried at 11_0. Everything else is forwarded. */
#include <windows.h>
#include <unknwn.h>

#define D3D_FEATURE_LEVEL_1_0_GENERIC 0x100
#define D3D_FEATURE_LEVEL_1_0_CORE    0x1000
#define D3D_FEATURE_LEVEL_11_0        0xb000

static HMODULE real;
static FARPROC fn(const char *name)
{
    if (!real) real = LoadLibraryA("d3d12_vkd3d.dll");
    return real ? GetProcAddress(real, name) : NULL;
}

HRESULT WINAPI D3D12CreateDevice(IUnknown *adapter, UINT level, REFIID iid, void **device)
{
    HRESULT (WINAPI *p)(IUnknown *, UINT, REFIID, void **) = (void *)fn("D3D12CreateDevice");
    HRESULT hr;

    if (!p) return E_FAIL;
    hr = p(adapter, level, iid, device);
    if (FAILED(hr) && (level == D3D_FEATURE_LEVEL_1_0_CORE || level == D3D_FEATURE_LEVEL_1_0_GENERIC))
        hr = p(adapter, D3D_FEATURE_LEVEL_11_0, iid, device);
    return hr;
}

#define FWD(name, args, call) HRESULT WINAPI name args { HRESULT (WINAPI *p) args = (void *)fn(#name); return p ? p call : E_FAIL; }
FWD(D3D12CreateRootSignatureDeserializer, (const void *a, SIZE_T b, REFIID c, void **d), (a, b, c, d))
FWD(D3D12CreateVersionedRootSignatureDeserializer, (const void *a, SIZE_T b, REFIID c, void **d), (a, b, c, d))
FWD(D3D12EnableExperimentalFeatures, (UINT a, const IID *b, void *c, UINT *d), (a, b, c, d))
FWD(D3D12GetDebugInterface, (REFIID a, void **b), (a, b))
FWD(D3D12GetInterface, (REFCLSID a, REFIID b, void **c), (a, b, c))
FWD(D3D12SerializeRootSignature, (const void *a, UINT b, void **c, void **d), (a, b, c, d))
FWD(D3D12SerializeVersionedRootSignature, (const void *a, void **b, void **c), (a, b, c))
