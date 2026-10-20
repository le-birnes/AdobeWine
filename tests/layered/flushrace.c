/* A thread presents a D3D11 swapchain in a child window (an offscreen client surface under
 * Wine) while the main thread draws on the toplevel with GDI and moves it. Each move flushes
 * the toplevel's window surface with the user lock held; patch 0019 re-presents the offscreen
 * surfaces from that flush, and before patch 0072 this deadlocked against the present.
 * A watchdog prints DEADLOCK when the main loop stops for 5 s.
 * Usage: flushrace.exe [seconds] */
#define COBJMACROS
#include <windows.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdlib.h>

static volatile LONG moves, presents, stop;

static DWORD WINAPI present_thread(void *arg)
{
    IDXGISwapChain *swapchain = arg;
    ID3D11Device *device; ID3D11DeviceContext *context;
    ID3D11Texture2D *backbuffer; ID3D11RenderTargetView *rtv;
    float color[4] = {0, 0, 1, 1};

    IDXGISwapChain_GetDevice(swapchain, &IID_ID3D11Device, (void **)&device);
    ID3D11Device_GetImmediateContext(device, &context);
    IDXGISwapChain_GetBuffer(swapchain, 0, &IID_ID3D11Texture2D, (void **)&backbuffer);
    ID3D11Device_CreateRenderTargetView(device, (ID3D11Resource *)backbuffer, NULL, &rtv);
    while (!stop)
    {
        color[1] = (presents & 1) ? 1.0f : 0.0f;
        ID3D11DeviceContext_ClearRenderTargetView(context, rtv, color);
        IDXGISwapChain_Present(swapchain, 0, 0);
        InterlockedIncrement(&presents);
    }
    return 0;
}

static DWORD WINAPI watchdog_thread(void *arg)
{
    LONG last = -1;
    int still = 0;

    while (!stop)
    {
        Sleep(1000);
        if (moves == last && ++still >= 5)
        {
            printf("DEADLOCK after %ld moves and %ld presents\n", moves, presents);
            fflush(stdout);
            TerminateProcess(GetCurrentProcess(), 2);
        }
        if (moves != last) still = 0;
        last = moves;
    }
    return 0;
}

int main(int argc, char **argv)
{
    DXGI_SWAP_CHAIN_DESC desc = {0};
    IDXGISwapChain *swapchain; ID3D11Device *device; ID3D11DeviceContext *context;
    HWND top, child; HRESULT hr; DWORD end;
    int seconds = argc > 1 ? atoi(argv[1]) : 20;

    top = CreateWindowA("static", "flushrace", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
                        100, 100, 400, 300, NULL, NULL, NULL, NULL);
    child = CreateWindowA("static", NULL, WS_CHILD | WS_VISIBLE, 50, 50, 200, 150, top, NULL, NULL, NULL);

    desc.BufferDesc.Width = 200; desc.BufferDesc.Height = 150;
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.OutputWindow = child;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    if (FAILED(hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
                                                  D3D11_SDK_VERSION, &desc, &swapchain, &device, NULL, &context)))
    {
        printf("D3D11CreateDeviceAndSwapChain %#lx\n", hr);
        return 1;
    }

    CreateThread(NULL, 0, present_thread, swapchain, 0, NULL);
    CreateThread(NULL, 0, watchdog_thread, NULL, 0, NULL);

    end = GetTickCount() + seconds * 1000;
    while (GetTickCount() < end)
    {
        MSG msg;
        HDC hdc = GetDC(top);
        RECT rc = {0, 0, 400, 40};

        FillRect(hdc, &rc, GetStockObject((moves & 1) ? BLACK_BRUSH : WHITE_BRUSH));  /* dirty the window surface */
        ReleaseDC(top, hdc);
        SetWindowPos(top, NULL, 100 + (moves & 7), 100, 400 + (moves & 1), 300, SWP_NOZORDER | SWP_NOACTIVATE);
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) DispatchMessageA(&msg);
        InterlockedIncrement(&moves);
    }
    stop = 1;

    printf("moves: %ld\n", moves);
    printf("presents: %ld\n", presents);
    printf("result: ok\n");
    fflush(stdout);
    TerminateProcess(GetCurrentProcess(), 0);  /* the present thread may still be inside Present */
    return 0;
}
