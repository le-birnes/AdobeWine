/* Prints the highest Direct3D 12 feature level the default adapter can create a device at,
 * and the level to report when that is below 12_0.
 * Photoshop's GPU check (sniffer.exe) creates its device at D3D_FEATURE_LEVEL_12_0 and
 * reports no GPU at all when that fails; Camera Raw refuses DirectX on an Intel GPU whose
 * highest level is below 12_1 ("pre-Skylake"). vkd3d-proton offers 12_0 only when the
 * Vulkan driver has sparse residency (tiled resources tier 2), which some drivers lack
 * although the Windows driver for the same GPU offers 12_x (e.g. Mesa ANV on Gen9 Intel
 * graphics). The level to report is 12_1 when the GPU has what 12_1 adds to 12_0 (ROVs and
 * conservative rasterization), as on Windows, and 12_0 otherwise.
 * Output: "<highest> <to report>", e.g. "11_1 12_1".
 * Exit code: 0 = 12_0 or higher, 2 = 11_x only (report the second level), 1 = no device. */
#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <stdio.h>
#include <d3d12.h>

int main(void)
{
    static const struct { D3D_FEATURE_LEVEL level; const char *name; } levels[] =
    {
        {D3D_FEATURE_LEVEL_12_2, "12_2"}, {D3D_FEATURE_LEVEL_12_1, "12_1"},
        {D3D_FEATURE_LEVEL_12_0, "12_0"}, {D3D_FEATURE_LEVEL_11_1, "11_1"},
        {D3D_FEATURE_LEVEL_11_0, "11_0"},
    };
    HRESULT (WINAPI *create)(IUnknown *, D3D_FEATURE_LEVEL, REFIID, void **);
    HMODULE d3d12 = LoadLibraryA("d3d12.dll");
    D3D12_FEATURE_DATA_D3D12_OPTIONS options = {0};
    ID3D12Device *device;
    unsigned int i;

    if (!d3d12 || !(create = (void *)GetProcAddress(d3d12, "D3D12CreateDevice")))
    {
        printf("none\n");
        return 1;
    }
    for (i = 0; i < ARRAYSIZE(levels); i++)
    {
        if (FAILED(create(NULL, levels[i].level, &IID_ID3D12Device, (void **)&device))) continue;
        if (levels[i].level >= D3D_FEATURE_LEVEL_12_0)
        {
            ID3D12Device_Release(device);
            printf("%s %s\n", levels[i].name, levels[i].name);
            return 0;
        }
        ID3D12Device_CheckFeatureSupport(device, D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
        ID3D12Device_Release(device);
        printf("%s %s\n", levels[i].name,
               options.ROVsSupported && options.ConservativeRasterizationTier ? "12_1" : "12_0");
        return 2;
    }
    printf("none\n");
    return 1;
}
