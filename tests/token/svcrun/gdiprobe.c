/* gdiprobe: does this process have GDI stock objects and can it create a window? Logs to argv[1]. */
#include <windows.h>
#include <stdio.h>
int main( int argc, char **argv )
{
    FILE *f = fopen( argc > 1 ? argv[1] : "C:\\gdiprobe.log", "w" );
    HWND w;
    char ws[256] = "", dt[256] = ""; DWORD n;
    GetUserObjectInformationA( GetProcessWindowStation(), UOI_NAME, ws, sizeof(ws), &n );
    GetUserObjectInformationA( GetThreadDesktop( GetCurrentThreadId() ), UOI_NAME, dt, sizeof(dt), &n );
    fprintf( f, "winsta %s desktop %s\n", ws, dt ); fflush( f );
    fprintf( f, "stock white brush %p font %p\n", GetStockObject( WHITE_BRUSH ), GetStockObject( SYSTEM_FONT ) ); fflush( f );
    w = CreateWindowA( "static", "x", 0, 0, 0, 10, 10, HWND_MESSAGE, 0, 0, 0 );
    fprintf( f, "message window %p error %lu\n", w, GetLastError() );
    fclose( f );
    return 0;
}
