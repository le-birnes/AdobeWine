/* GDI+ bitmap over caller memory with a padded stride (Illustrator: 46x14, stride 192,
 * PixelFormat32bppRGB), drawn onto a DIB-section HDC: every row must land unshifted. */
#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>

int main(void)
{
    GdiplusStartupInput in = {1}; ULONG_PTR tok;
    GpBitmap *bmp; GpGraphics *g; HDC dc; HBITMAP dib; DWORD *dst;
    static DWORD src[14 * 48];
    BITMAPINFO bi = {{sizeof(BITMAPINFOHEADER), 64, -20, 1, 32, BI_RGB}};
    int x, y, bad = 0, mode;
    GpPointF pts[3] = {{0, 0}, {46, 0}, {0, 14}};
    PixelFormat fmts[2] = {PixelFormat32bppRGB, PixelFormat32bppPARGB};

    GdiplusStartup(&tok, &in, NULL);
    for (y = 0; y < 14; y++) for (x = 0; x < 48; x++)
        src[y * 48 + x] = x < 46 ? (0xff000000 | (y << 16) | (x << 8) | 0x7f) : 0xffff00ff; /* padding = magenta */

    for (mode = 0; mode < 4; mode++)
    {
        dc = CreateCompatibleDC(0);
        dib = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void **)&dst, NULL, 0);
        SelectObject(dc, dib);
        memset(dst, 0, 64 * 20 * 4);
        GdipCreateBitmapFromScan0(46, 14, 192, fmts[mode & 1], (BYTE *)src, &bmp);
        GdipCreateFromHDC(dc, &g);
        if (mode < 2) GdipDrawImagePointsRect(g, (GpImage *)bmp, pts, 3, 0, 0, 46, 14, UnitPixel, NULL, NULL, NULL);
        else GdipDrawImageRectRect(g, (GpImage *)bmp, 0, 0, 46, 14, 0, 0, 46, 14, UnitPixel, NULL, NULL, NULL);
        GdipDeleteGraphics(g);
        bad = 0;
        for (y = 0; y < 14; y++) for (x = 0; x < 46; x++)
            if ((dst[y * 64 + x] & 0xffffff) != (src[y * 48 + x] & 0xffffff)) bad++;
        printf("mode %d (%s, %s): %d wrong pixels; row 5 starts with %06lx (expected %06lx)\n", mode,
               mode < 2 ? "DrawImagePointsRect" : "DrawImageRectRect", mode & 1 ? "PARGB" : "RGB",
               bad, dst[5 * 64] & 0xffffff, src[5 * 48] & 0xffffff);
        GdipDisposeImage((GpImage *)bmp); DeleteDC(dc); DeleteObject(dib);
    }
    return 0;
}
