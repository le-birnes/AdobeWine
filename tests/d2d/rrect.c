/* Draw rounded-rectangle outlines with Direct2D into a WIC bitmap and save rrect.png.
 * Reference for the "bone-shaped button" outline bug seen in Adobe apps. */
#define COBJMACROS
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <stdio.h>

static IWICImagingFactory *wic;

static void save(IWICBitmap *bmp, const WCHAR *path)
{
    IWICStream *stream; IWICBitmapEncoder *enc; IWICBitmapFrameEncode *frame;
    IWICImagingFactory_CreateStream(wic, &stream);
    IWICStream_InitializeFromFilename(stream, path, GENERIC_WRITE);
    IWICImagingFactory_CreateEncoder(wic, &GUID_ContainerFormatPng, NULL, &enc);
    IWICBitmapEncoder_Initialize(enc, (IStream *)stream, WICBitmapEncoderNoCache);
    IWICBitmapEncoder_CreateNewFrame(enc, &frame, NULL);
    IWICBitmapFrameEncode_Initialize(frame, NULL);
    IWICBitmapFrameEncode_WriteSource(frame, (IWICBitmapSource *)bmp, NULL);
    IWICBitmapFrameEncode_Commit(frame);
    IWICBitmapEncoder_Commit(enc);
}

int main(void)
{
    ID2D1Factory *f; IWICBitmap *bmp; ID2D1RenderTarget *rt; ID2D1SolidColorBrush *br;
    D2D1_RENDER_TARGET_PROPERTIES p = {0};
    D2D1_COLOR_F white = {1, 1, 1, 1}, black = {0, 0, 0, 1};
    D2D1_STROKE_STYLE_PROPERTIES sp = {D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT,
                                       D2D1_LINE_JOIN_MITER, 10.0f, D2D1_DASH_STYLE_SOLID, 0.0f};
    ID2D1StrokeStyle *ss;
    float widths[] = {1.0f, 2.0f, 4.0f};
    int i;

    CoInitialize(NULL);
    CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, (void **)&wic);
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, (void **)&f);
    IWICImagingFactory_CreateBitmap(wic, 400, 380, &GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &bmp);
    p.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM; p.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    ID2D1Factory_CreateWicBitmapRenderTarget(f, bmp, &p, &rt);
    ID2D1Factory_CreateStrokeStyle(f, &sp, NULL, 0, &ss);
    ID2D1RenderTarget_CreateSolidColorBrush(rt, &black, NULL, &br);
    ID2D1RenderTarget_BeginDraw(rt);
    ID2D1RenderTarget_Clear(rt, &white);
    for (i = 0; i < 3; i++)
    {
        D2D1_ROUNDED_RECT r = {{20.5f, 20.5f + i * 90, 180.5f, 80.5f + i * 90}, 30.0f, 30.0f};
        ID2D1RenderTarget_DrawRoundedRectangle(rt, &r, (ID2D1Brush *)br, widths[i], NULL);
        r.rect.left += 200; r.rect.right += 200;
        ID2D1RenderTarget_DrawRoundedRectangle(rt, &r, (ID2D1Brush *)br, widths[i], ss);
    }
    /* Row 4: path geometry built from lines and arcs, as UI toolkits do; row 5: same with a 2x transform */
    {
        int k;
        for (k = 0; k < 2; k++)
        {
            ID2D1PathGeometry *g; ID2D1GeometrySink *sk;
            float x0 = 20.5f + k * 200, y0 = 290.5f, x1 = x0 + 80, y1 = y0 + 20, rr = 10.0f;
            D2D1_ARC_SEGMENT a = {{0}, {rr, rr}, 90.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL};
            D2D1_POINT_2F pt;
            D2D1_MATRIX_3X2_F m = {1, 0, 0, 1, 0, 0};
            ID2D1Factory_CreatePathGeometry(f, &g);
            ID2D1PathGeometry_Open(g, &sk);
            /* Adobe's pattern: start at the left middle, zero-length line, arc, line, arc, zero-length line... */
            pt.x = x0; pt.y = y0 + rr; ID2D1GeometrySink_BeginFigure(sk, pt, D2D1_FIGURE_BEGIN_FILLED);
            ID2D1GeometrySink_AddLine(sk, pt);
            a.point.x = x0 + rr; a.point.y = y0; ID2D1GeometrySink_AddArc(sk, &a);
            pt.x = x1 - rr; pt.y = y0; ID2D1GeometrySink_AddLine(sk, pt);
            a.point.x = x1; a.point.y = y0 + rr; ID2D1GeometrySink_AddArc(sk, &a);
            pt.x = x1; pt.y = y0 + rr; ID2D1GeometrySink_AddLine(sk, pt);
            a.point.x = x1 - rr; a.point.y = y1; ID2D1GeometrySink_AddArc(sk, &a);
            pt.x = x0 + rr; pt.y = y1; ID2D1GeometrySink_AddLine(sk, pt);
            a.point.x = x0; a.point.y = y0 + rr; ID2D1GeometrySink_AddArc(sk, &a);
            ID2D1GeometrySink_EndFigure(sk, D2D1_FIGURE_END_CLOSED);
            ID2D1GeometrySink_Close(sk);
            if (k) { m._31 = 0.5f; m._32 = 0.5f; }  /* half scale */
            ID2D1RenderTarget_SetTransform(rt, &m);
            ID2D1RenderTarget_DrawGeometry(rt, (ID2D1Geometry *)g, (ID2D1Brush *)br, 1.0f, k ? ss : NULL);
        }
        {
            D2D1_MATRIX_3X2_F id = {1, 0, 0, 1, 0, 0};
            ID2D1RenderTarget_SetTransform(rt, &id);
        }
    }
    printf("EndDraw %#lx\n", ID2D1RenderTarget_EndDraw(rt, NULL, NULL));
    save(bmp, L"rrect.png");
    return 0;
}
