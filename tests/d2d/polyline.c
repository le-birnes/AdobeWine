/* Stroke a densely sampled polyline (Audition's Parametric Equalizer curve, 569 points, one
 * per pixel, .5 coordinates) with width 1.6 / miter limit 3, as Audition does, and count
 * drawn pixels farther than 3 px from the polyline. Wine drew 25-unit tangent spikes wherever
 * consecutive segments were exactly collinear. Usage: polyline.exe eqcurve.txt */
#define COBJMACROS
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <math.h>
#include <stdio.h>

static D2D1_POINT_2F pts[1024];
static unsigned int count;

static float seg_dist(float px, float py, D2D1_POINT_2F a, D2D1_POINT_2F b)
{
    float dx = b.x - a.x, dy = b.y - a.y, t = ((px - a.x) * dx + (py - a.y) * dy) / (dx * dx + dy * dy);
    if (t < 0) t = 0; if (t > 1) t = 1;
    return hypotf(px - (a.x + t * dx), py - (a.y + t * dy));
}

int main(int argc, char **argv)
{
    D2D1_RENDER_TARGET_PROPERTIES props = {0};
    D2D1_STROKE_STYLE_PROPERTIES ssp = {0};
    ID2D1Factory *factory; IWICImagingFactory *wic; IWICBitmap *bmp; IWICBitmapLock *lock;
    ID2D1RenderTarget *rt; ID2D1PathGeometry *path; ID2D1GeometrySink *sink;
    ID2D1SolidColorBrush *brush; ID2D1StrokeStyle *style;
    D2D1_COLOR_F white = {1, 1, 1, 1}, black = {0, 0, 0, 1};
    WICRect rect = {0, 0, 600, 240}; UINT stride, size; BYTE *data;
    unsigned int x, y, i, drawn = 0, off_curve = 0;
    FILE *fp = fopen(argc > 1 ? argv[1] : "eqcurve.txt", "r");

    if (!fp) { printf("no points file\n"); return 1; }
    while (count < 1024 && fscanf(fp, "%f %f", &pts[count].x, &pts[count].y) == 2) count++;
    fclose(fp);

    CoInitialize(NULL);
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, (void **)&factory);
    CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, (void **)&wic);
    IWICImagingFactory_CreateBitmap(wic, 600, 240, &GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &bmp);
    props.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM; props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    ID2D1Factory_CreateWicBitmapRenderTarget(factory, bmp, &props, &rt);
    ID2D1Factory_CreatePathGeometry(factory, &path);
    ID2D1PathGeometry_Open(path, &sink);
    ID2D1GeometrySink_BeginFigure(sink, pts[0], D2D1_FIGURE_BEGIN_HOLLOW);
    ID2D1GeometrySink_AddLines(sink, pts + 1, count - 1);
    ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_OPEN);
    ID2D1GeometrySink_Close(sink);
    ssp.lineJoin = D2D1_LINE_JOIN_MITER; ssp.miterLimit = 3.0f;
    ID2D1Factory_CreateStrokeStyle(factory, &ssp, NULL, 0, &style);
    ID2D1RenderTarget_CreateSolidColorBrush(rt, &black, NULL, &brush);
    ID2D1RenderTarget_BeginDraw(rt);
    ID2D1RenderTarget_Clear(rt, &white);
    ID2D1RenderTarget_DrawGeometry(rt, (ID2D1Geometry *)path, (ID2D1Brush *)brush, 1.6f, style);
    ID2D1RenderTarget_EndDraw(rt, NULL, NULL);

    IWICBitmap_Lock(bmp, &rect, WICBitmapLockRead, &lock);
    IWICBitmapLock_GetStride(lock, &stride); IWICBitmapLock_GetDataPointer(lock, &size, &data);
    for (y = 0; y < 240; y++) for (x = 0; x < 600; x++)
    {
        float d = 1e9f;
        if (data[y * stride + x * 4 + 1] > 128) continue; /* background */
        drawn++;
        for (i = 0; i + 1 < count && d > 3.0f; i++) d = fminf(d, seg_dist(x + 0.5f, y + 0.5f, pts[i], pts[i + 1]));
        if (d > 3.0f) off_curve++;
    }
    printf("%u points, %u pixels drawn, %u farther than 3 px from the curve: %s\n", count, drawn, off_curve, off_curve ? "FAIL" : "PASS");
    return off_curve != 0;
}
