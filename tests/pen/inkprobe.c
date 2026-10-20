/* inkprobe: log what a Windows Ink application sees from a pen.
 * A window covering most of the screen; every WM_POINTER* message is written to the file
 * given on the command line as "msg type pressure tiltX tiltY flags x y".
 * Build: x86_64-w64-mingw32-gcc -O2 -o inkprobe.exe inkprobe.c -mwindows */
#define _WIN32_WINNT 0x0602
#include <windows.h>
#include <stdio.h>

static FILE *out;

static LRESULT CALLBACK wndproc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
    switch (msg)
    {
    case WM_POINTERDOWN:
    case WM_POINTERUPDATE:
    case WM_POINTERUP:
    {
        UINT32 id = GET_POINTERID_WPARAM( wparam );
        POINTER_INPUT_TYPE type = PT_POINTER;
        POINTER_PEN_INFO pen = {0};

        GetPointerType( id, &type );
        if (type == PT_PEN && GetPointerPenInfo( id, &pen ))
            fprintf( out, "%#x pen %u %d %d %#x %ld %ld\n", msg, pen.pressure, pen.tiltX, pen.tiltY,
                     (unsigned)pen.penFlags, pen.pointerInfo.ptPixelLocation.x, pen.pointerInfo.ptPixelLocation.y );
        else
            fprintf( out, "%#x type %d\n", msg, (int)type );
        fflush( out );
        break;
    }
    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;
    }
    return DefWindowProcW( hwnd, msg, wparam, lparam );
}

int WINAPI WinMain( HINSTANCE instance, HINSTANCE prev, LPSTR cmdline, int show )
{
    WNDCLASSW wc = {0};
    MSG msg;
    HWND hwnd;

    if (!(out = fopen( cmdline && *cmdline ? cmdline : "inkprobe.log", "w" ))) return 1;
    wc.lpfnWndProc = wndproc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW( NULL, (LPCWSTR)IDC_ARROW );
    wc.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
    wc.lpszClassName = L"inkprobe";
    RegisterClassW( &wc );
    hwnd = CreateWindowW( L"inkprobe", L"inkprobe", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                          100, 100, 1600, 900, NULL, NULL, instance, NULL );
    SetForegroundWindow( hwnd );
    while (GetMessageW( &msg, NULL, 0, 0 ))
    {
        TranslateMessage( &msg );
        DispatchMessageW( &msg );
    }
    fclose( out );
    return 0;
}
