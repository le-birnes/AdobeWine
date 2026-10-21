/* mkfontkey NAME: leave an empty family key in Wine's volatile font cache, like a font that another process
 * added with AddFontResource and removed again (Creative Cloud's Adobe Fonts sync). */
#include <windows.h>
#include <stdio.h>
int main( int argc, char **argv )
{
    HKEY key; char path[256]; LONG r;
    snprintf( path, sizeof(path), "Software\\Wine\\Fonts\\Cache\\%s", argc > 1 ? argv[1] : "Open Sans" );
    r = RegCreateKeyExA( HKEY_CURRENT_USER, path, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL );
    printf( "create %s: %ld\n", path, r );
    return r;
}
