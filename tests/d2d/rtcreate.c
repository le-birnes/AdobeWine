/* Creates many WIC bitmap render targets, draws a rectangle in each and times them. Every
 * render target gets its own d2d device; Wine compiled the shape shaders for each one (patch
 * 0073 compiles them once). Also checks that every render target still draws correctly.
 * Usage: rtcreate.exe [count] */
#define COBJMACROS
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <stdio.h>
#include <stdlib.h>

static double now_ms(void)
{
    LARGE_INTEGER c, f;
    QueryPerformanceCounter(&c); QueryPerformanceFrequency(&f);
    return c.QuadPart * 1000.0 / f.QuadPart;
}

int main(int argc, char **argv)
{
    IWICImagingFactory *wic; ID2D1Factory *f; IWICBitmap *bmp;
    D2D1_RENDER_TARGET_PROPERTIES p = {0};
    D2D1_COLOR_F white = {1, 1, 1, 1}, red = {1, 0, 0, 1};
    D2D1_RECT_F r = {8, 8, 24, 24};
    WICRect wr = {0, 0, 32, 32};
    int count = argc > 1 ? atoi(argv[1]) : 50, fails = 0, i;
    double first = 0, start, t;

    CoInitialize(NULL);
    CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, (void **)&wic);
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, (void **)&f);
    IWICImagingFactory_CreateBitmap(wic, 32, 32, &GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &bmp);
    p.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM; p.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

    start = now_ms();
    for (i = 0; i < count; i++)
    {
        ID2D1RenderTarget *rt; ID2D1SolidColorBrush *brush; IWICBitmapLock *lock;
        UINT size, stride; BYTE *data; HRESULT hr;

        t = now_ms();
        if (FAILED(hr = ID2D1Factory_CreateWicBitmapRenderTarget(f, bmp, &p, &rt)))
        {
            printf("render target %d: CreateWicBitmapRenderTarget %#lx\n", i, hr);
            return 1;
        }
        ID2D1RenderTarget_CreateSolidColorBrush(rt, &red, NULL, &brush);
        ID2D1RenderTarget_BeginDraw(rt);
        ID2D1RenderTarget_Clear(rt, &white);
        ID2D1RenderTarget_FillRectangle(rt, &r, (ID2D1Brush *)brush);
        hr = ID2D1RenderTarget_EndDraw(rt, NULL, NULL);
        ID2D1SolidColorBrush_Release(brush);
        ID2D1RenderTarget_Release(rt);
        if (!i) first = now_ms() - t;

        IWICBitmap_Lock(bmp, &wr, WICBitmapLockRead, &lock);
        IWICBitmapLock_GetStride(lock, &stride);
        IWICBitmapLock_GetDataPointer(lock, &size, &data);
        /* BGRA: (16,16) red inside the rectangle, (2,2) white outside it */
        if (hr || *(DWORD *)(data + 16 * stride + 16 * 4) != 0xffff0000 || *(DWORD *)(data + 2 * stride + 2 * 4) != 0xffffffff)
        {
            if (fails++ < 5)
                printf("render target %d: EndDraw %#lx, inside %08lx, outside %08lx\n", i, hr,
                       *(DWORD *)(data + 16 * stride + 16 * 4), *(DWORD *)(data + 2 * stride + 2 * 4));
        }
        IWICBitmapLock_Release(lock);
    }
    t = now_ms() - start;

    printf("render targets: %d\n", count);
    printf("first: %.1f ms\n", first);
    printf("average: %.2f ms\n", t / count);
    printf("pixels: %s (%d wrong)\n", fails ? "FAIL" : "ok", fails);
    return fails != 0;
}
