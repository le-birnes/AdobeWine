/* How does CreateBitmap/SetBitmapBits lay out 32bpp rows?  Fill a buffer whose
 * pixel value is (row << 16 | col), create a 313-wide 32bpp DDB from it, and
 * read rows back with GetDIBits (DWORD-aligned, top-down). */
#include <windows.h>
#include <stdio.h>
int main(void)
{
    int widths[] = { 313, 315, 2461, 64 }, i;
    for (i = 0; i < 4; i++)
    {
        int w = widths[i], h = 4, x, y;
        DWORD *buf = calloc(w * 2 * h, 4), *out = calloc(w * h, 4);
        BITMAP bm; BITMAPINFO bi = {{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB}};
        HBITMAP hb; HDC dc = GetDC(0); LONG n;
        for (y = 0; y < 2 * h; y++) for (x = 0; x < w; x++) buf[y * w + x] = (y << 16) | x;
        for (x = 0; x < w * 2 * h; x++) buf[x] = ((x / w) << 16) | (x % w);
        hb = CreateBitmap(w, h, 1, 32, buf);
        GetObjectA(hb, sizeof(bm), &bm);
        GetDIBits(dc, hb, 0, h, out, &bi, DIB_RGB_COLORS);
        printf("w %d: bmWidthBytes %d; row1 starts with linear index %lu\n", w, bm.bmWidthBytes,
               (unsigned long)((out[w] >> 16) * w + (out[w] & 0xffff)));
        hb = CreateBitmap(w, h, 1, 32, NULL);
        n = SetBitmapBits(hb, w * h * 4, buf);
        GetDIBits(dc, hb, 0, h, out, &bi, DIB_RGB_COLORS);
        printf("   SetBitmapBits ret %ld; row1 starts at %lu\n", n,
               (unsigned long)((out[w] >> 16) * w + (out[w] & 0xffff)));
        hb = CreateCompatibleBitmap(dc, w, h);
        GetObjectA(hb, sizeof(bm), &bm);
        printf("   compatible: bpp %d widthbytes %d\n", bm.bmBitsPixel, bm.bmWidthBytes);
    }
    return 0;
}
