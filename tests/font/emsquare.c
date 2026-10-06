/* emsquare.exe <font file> <face>: load a private font and print what GDI reports for it
 * (otmEMSquare, ascent, glyph outline of 'A' at the em size, GetFontData table sizes). */
#include <windows.h>
#include <stdio.h>
int main(int argc, char **argv)
{
    int n = AddFontResourceExA(argv[1], FR_PRIVATE, 0);
    HDC dc = CreateCompatibleDC(0);
    LOGFONTA lf = {0}; HFONT f; OUTLINETEXTMETRICA *otm; UINT sz; GLYPHMETRICS gm; MAT2 m = {{0,1},{0,0},{0,0},{0,1}};
    lf.lfHeight = -1000; strcpy(lf.lfFaceName, argv[2]);
    f = CreateFontIndirectA(&lf); SelectObject(dc, f);
    sz = GetOutlineTextMetricsA(dc, 0, NULL); otm = malloc(sz); GetOutlineTextMetricsA(dc, sz, otm);
    {
        char face[64]; GetTextFaceA(dc, 64, face);
        printf("added %d face '%s' emsquare %u ascent %d descent %d\n", n, face, otm->otmEMSquare, otm->otmAscent, otm->otmDescent);
    }
    GetGlyphOutlineA(dc, 'A', GGO_NATIVE, &gm, 0, NULL, &m);
    printf("A at 1000: black %ux%u origin %ld,%ld advance %d\n", gm.gmBlackBoxX, gm.gmBlackBoxY, gm.gmptGlyphOrigin.x, gm.gmptGlyphOrigin.y, gm.gmCellIncX);
    printf("CFF table %ld, glyf %ld, head %ld, whole %ld\n", (long)GetFontData(dc, 0x20464643, 0, NULL, 0),
           (long)GetFontData(dc, 0x66796c67, 0, NULL, 0), (long)GetFontData(dc, 0x64616568, 0, NULL, 0), (long)GetFontData(dc, 0, 0, NULL, 0));
    return 0;
}
