/* PushLayer with a geometric mask (two rectangles) and with opacity: checks pixels. */
#define COBJMACROS
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <stdio.h>

int main(void)
{
    IWICImagingFactory *wic; ID2D1Factory *f; IWICBitmap *bmp; ID2D1RenderTarget *rt;
    ID2D1SolidColorBrush *red, *blue; ID2D1RectangleGeometry *r1, *r2; ID2D1GeometryGroup *grp;
    ID2D1Geometry *list[2]; IWICBitmapLock *lock; UINT size, stride; BYTE *data;
    D2D1_RENDER_TARGET_PROPERTIES p = {0};
    D2D1_COLOR_F white = {1, 1, 1, 1}, cr = {1, 0, 0, 1}, cb = {0, 0, 1, 1};
    D2D1_RECT_F a = {10, 10, 50, 50}, b = {100, 10, 140, 50}, full = {0, 0, 200, 100};
    D2D1_LAYER_PARAMETERS lp = {{-1e30f, -1e30f, 1e30f, 1e30f}, NULL, D2D1_ANTIALIAS_MODE_ALIASED,
                                {1, 0, 0, 1, 0, 0}, 1.0f, NULL, D2D1_LAYER_OPTIONS_NONE};
    WICRect wr = {0, 0, 200, 100};
    int fails = 0;

    CoInitialize(NULL);
    CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, (void **)&wic);
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, (void **)&f);
    IWICImagingFactory_CreateBitmap(wic, 200, 100, &GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &bmp);
    p.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM; p.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    ID2D1Factory_CreateWicBitmapRenderTarget(f, bmp, &p, &rt);
    ID2D1RenderTarget_CreateSolidColorBrush(rt, &cr, NULL, &red);
    ID2D1RenderTarget_CreateSolidColorBrush(rt, &cb, NULL, &blue);
    ID2D1Factory_CreateRectangleGeometry(f, &a, &r1);
    ID2D1Factory_CreateRectangleGeometry(f, &b, &r2);
    list[0] = (ID2D1Geometry *)r1; list[1] = (ID2D1Geometry *)r2;
    ID2D1Factory_CreateGeometryGroup(f, D2D1_FILL_MODE_WINDING, list, 2, &grp);

    ID2D1RenderTarget_BeginDraw(rt);
    ID2D1RenderTarget_Clear(rt, &white);
    lp.geometricMask = (ID2D1Geometry *)grp;
    ID2D1RenderTarget_PushLayer(rt, &lp, NULL);
    ID2D1RenderTarget_FillRectangle(rt, &full, (ID2D1Brush *)red);   /* only inside a and b */
    ID2D1RenderTarget_PopLayer(rt);
    lp.geometricMask = NULL; lp.opacity = 0.5f;
    {
        D2D1_RECT_F c = {160, 60, 190, 90};
        ID2D1RenderTarget_PushLayer(rt, &lp, NULL);
        ID2D1RenderTarget_FillRectangle(rt, &c, (ID2D1Brush *)blue);  /* half-strength blue */
        ID2D1RenderTarget_PopLayer(rt);
    }
    printf("EndDraw %#lx\n", ID2D1RenderTarget_EndDraw(rt, NULL, NULL));

    IWICBitmap_Lock(bmp, &wr, WICBitmapLockRead, &lock);
    IWICBitmapLock_GetStride(lock, &stride);
    IWICBitmapLock_GetDataPointer(lock, &size, &data);
#define PX(x, y) (*(DWORD *)(data + (y) * stride + (x) * 4))
    {
        struct { int x, y; DWORD expect; const char *what; } t[] = {
            {30, 30, 0xffff0000, "inside mask a"}, {120, 30, 0xffff0000, "inside mask b"},
            {75, 30, 0xffffffff, "between the mask rects"}, {30, 80, 0xffffffff, "below mask"},
            {175, 75, 0xff7f7fff, "opacity layer"}, {150, 75, 0xffffffff, "outside opacity rect"}};
        unsigned int i;
        for (i = 0; i < ARRAYSIZE(t); i++)
        {
            DWORD v = PX(t[i].x, t[i].y);
            int ok = abs((int)(v & 0xff) - (int)(t[i].expect & 0xff)) <= 2
                  && abs((int)((v >> 8) & 0xff) - (int)((t[i].expect >> 8) & 0xff)) <= 2
                  && abs((int)((v >> 16) & 0xff) - (int)((t[i].expect >> 16) & 0xff)) <= 2;
            printf("%-24s %08lx expect %08lx %s\n", t[i].what, v, t[i].expect, ok ? "ok" : "FAIL");
            fails += !ok;
        }
    }
    printf("%d failures\n", fails);
    return fails;
}
