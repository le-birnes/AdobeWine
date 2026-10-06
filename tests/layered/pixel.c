/* Print surface pixels of a window given as hex handle: pixel.exe <hwnd> */
#include <windows.h>
#include <stdio.h>
int main(int argc, char **argv)
{
    HWND hwnd = (HWND)(ULONG_PTR)strtoull(argv[1], NULL, 16);
    HDC dc = GetWindowDC(hwnd);
    int pts[][2] = {{0,0},{5,5},{100,100},{250,500},{499,999}};
    char cls[64]; HBRUSH br = (HBRUSH)GetClassLongPtrA(hwnd, GCLP_HBRBACKGROUND);
    GetClassNameA(hwnd, cls, sizeof(cls));
    printf("class %s brush %p sys3dface %06lx\n", cls, br, GetSysColor(COLOR_3DFACE));
    for (int i = 0; i < 5; i++) printf("(%d,%d) %06lx\n", pts[i][0], pts[i][1], GetPixel(dc, pts[i][0], pts[i][1]));
    ReleaseDC(hwnd, dc);
    return 0;
}
