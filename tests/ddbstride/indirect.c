/* Does CreateBitmapIndirect honour BITMAP.bmWidthBytes?  Illustrator passes
 * 32bpp bits with 16-byte aligned rows (313 px wide, 1264 bytes per row). */
#include <windows.h>
#include <stdio.h>
static void test(int w, int bpp, int wb)
{
    int h = 4, x, ok = 1, y;
    BYTE *buf = calloc(wb, h + 1);
    DWORD *out = calloc(w * h, 4);
    BITMAP bm = { 0, w, h, wb, 1, bpp, buf }, got;
    BITMAPINFO bi = {{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB}};
    HDC dc = GetDC(0);
    HBITMAP hb;
    for (y = 0; y < h; y++) for (x = 0; x < w && x * 4 < wb; x++)
        if (bpp == 32) ((DWORD *)(buf + y * wb))[x] = (y << 16) | x;
    hb = CreateBitmapIndirect(&bm);
    GetObjectA(hb, sizeof(got), &got);
    if (bpp == 32)
    {
        GetDIBits(dc, hb, 0, h, out, &bi, DIB_RGB_COLORS);
        for (y = 0; y < h; y++) for (x = 0; x < w; x++)
            if ((out[y * w + x] & 0xffffff) != (DWORD)((y << 16) | x)) ok = 0;
    }
    printf("w %d bpp %d bmWidthBytes in %d: hb %p, GetObject widthbytes %ld, rows %s\n",
           w, bpp, wb, hb, (long)got.bmWidthBytes, bpp == 32 ? (ok ? "follow bmWidthBytes" : "do NOT follow bmWidthBytes") : "-");
}
int main(void)
{
    test(313, 32, 1264);
    test(313, 32, 1252);
    test(2461, 32, 9856);
    test(313, 32, 1260);
    test(313, 32, 1254);
    test(313, 32, 1200);   /* smaller than needed */
    test(313, 32, 1253);   /* odd */
    test(313, 32, 0);
    test(10, 1, 4);
    test(10, 8, 12);
    return 0;
}
