/* sharedfence: a fence shared between D3D11 and D3D12 through an NT handle (as After Effects' viewer does).
 * D3D11 signals, D3D12 must see the value; D3D12 signals, D3D11 must see it. Prints PASS/FAIL to stdout.
 * Build: x86_64-w64-mingw32-gcc -O2 -o sharedfence.exe sharedfence.c -ld3d11 -ld3d12 -ldxgi -luuid */
#define COBJMACROS
#include <windows.h>
#include <initguid.h>
#include <d3d11_4.h>
#include <d3d12.h>
#include <stdio.h>

static UINT64 wait_value( UINT64 (*get)(void *), void *obj, UINT64 want )
{
    DWORD start = GetTickCount(); UINT64 v;
    while ((v = get( obj )) < want && GetTickCount() - start < 2000) Sleep( 10 );
    return v;
}
static UINT64 get11( void *f ) { return ID3D11Fence_GetCompletedValue( (ID3D11Fence *)f ); }
static UINT64 get12( void *f ) { return ID3D12Fence_GetCompletedValue( (ID3D12Fence *)f ); }

int main(void)
{
    ID3D11Device *d11; ID3D11Device5 *d11_5; ID3D11DeviceContext *ctx; ID3D11DeviceContext4 *ctx4;
    ID3D11Fence *f11, *f11b; ID3D12Device *d12; ID3D12CommandQueue *q; ID3D12Fence *f12, *f12b;
    D3D12_COMMAND_QUEUE_DESC qd = {0}; HANDLE h; HRESULT hr; UINT64 v; int fail = 0;

    hr = D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &d11, NULL, &ctx );
    if (FAILED(hr)) { printf( "d3d11 device %#lx\n", hr ); return 1; }
    ID3D11Device_QueryInterface( d11, &IID_ID3D11Device5, (void **)&d11_5 );
    ID3D11DeviceContext_QueryInterface( ctx, &IID_ID3D11DeviceContext4, (void **)&ctx4 );
    if (FAILED(hr = D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, (void **)&d12 ))) { printf( "d3d12 device %#lx\n", hr ); return 1; }
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ID3D12Device_CreateCommandQueue( d12, &qd, &IID_ID3D12CommandQueue, (void **)&q );

    /* D3D11 -> D3D12 */
    hr = ID3D11Device5_CreateFence( d11_5, 0, D3D11_FENCE_FLAG_SHARED, &IID_ID3D11Fence, (void **)&f11 );
    printf( "d3d11 CreateFence %#lx\n", hr );
    hr = ID3D11Fence_CreateSharedHandle( f11, NULL, GENERIC_ALL, NULL, &h );
    printf( "d3d11 CreateSharedHandle %#lx %p\n", hr, h );
    hr = ID3D12Device_OpenSharedHandle( d12, h, &IID_ID3D12Fence, (void **)&f12 );
    printf( "d3d12 OpenSharedHandle %#lx\n", hr );
    if (SUCCEEDED(hr))
    {
        ID3D11DeviceContext4_Signal( ctx4, f11, 5 ); ID3D11DeviceContext_Flush( ctx );
        v = wait_value( get12, f12, 5 );
        printf( "d3d11 signal 5 -> d3d12: %s\n", v >= 5 ? "PASS" : "FAIL" );
        printf( "  value %llu\n", (unsigned long long)v );
        fail |= v < 5;
        ID3D12CommandQueue_Signal( q, f12, 9 );
        v = wait_value( get11, f11, 9 );
        printf( "d3d12 signal 9 on the opened fence -> d3d11 sees %llu: %s\n", (unsigned long long)v, v >= 9 ? "PASS" : "FAIL" );
        fail |= v < 9;
    }
    else fail = 1;

    /* D3D12 -> D3D11 */
    hr = ID3D12Device_CreateFence( d12, 0, D3D12_FENCE_FLAG_SHARED, &IID_ID3D12Fence, (void **)&f12b );
    hr = ID3D12Device_CreateSharedHandle( d12, (ID3D12DeviceChild *)f12b, NULL, GENERIC_ALL, NULL, &h );
    printf( "d3d12 CreateSharedHandle %#lx %p\n", hr, h );
    hr = ID3D11Device5_OpenSharedFence( d11_5, h, &IID_ID3D11Fence, (void **)&f11b );
    printf( "d3d11 OpenSharedFence %#lx\n", hr );
    if (SUCCEEDED(hr))
    {
        ID3D12CommandQueue_Signal( q, f12b, 3 );
        v = wait_value( get11, f11b, 3 );
        printf( "d3d12 signal 3 -> d3d11 sees %llu: %s\n", (unsigned long long)v, v >= 3 ? "PASS" : "FAIL" );
        fail |= v < 3;
    }
    else fail = 1;
    printf( "%s\n", fail ? "FAIL" : "PASS" );
    return fail;
}
