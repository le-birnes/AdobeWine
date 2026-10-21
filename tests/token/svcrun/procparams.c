/* procparams PID: print another process's RTL_USER_PROCESS_PARAMETERS (flags, strings, environment size and
 * the variables) read from its PEB. */
#include <windows.h>
#include <winternl.h>
#include <stdio.h>
typedef struct { ULONG max, len, flags, debug; HANDLE console; ULONG cflags; HANDLE in, out, err; UNICODE_STRING curdir; HANDLE curdirh;
  UNICODE_STRING dllpath, image, cmdline; WCHAR *env; ULONG x, y, xs, ys, xc, yc, fill, dwFlags, show; UNICODE_STRING title, desktop, shell, runtime;
  BYTE dirs[32 * 24]; ULONG_PTR envsize; } PARAMS;
static void str( HANDLE h, const char *name, UNICODE_STRING *s )
{
    WCHAR buf[4096] = {0}; SIZE_T n;
    ReadProcessMemory( h, s->Buffer, buf, min( s->Length, sizeof(buf) - 2 ), &n );
    printf( "%s (%u bytes) '%ls'\n", name, s->Length, buf );
}
int main( int argc, char **argv )
{
    HANDLE h = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, strtoul( argv[1], NULL, 0 ) );
    PROCESS_BASIC_INFORMATION pbi; PEB peb; PARAMS p; SIZE_T n; WCHAR *env, *e;
    if (!h) { printf( "OpenProcess %lu\n", GetLastError() ); return 1; }
    NtQueryInformationProcess( h, ProcessBasicInformation, &pbi, sizeof(pbi), NULL );
    ReadProcessMemory( h, pbi.PebBaseAddress, &peb, sizeof(peb), &n );
    ReadProcessMemory( h, peb.ProcessParameters, &p, sizeof(p), &n );
    printf( "params max %lu len %lu flags %#lx console %p cflags %#lx in %p out %p err %p dwFlags %#lx show %lu envsize %Iu\n",
            p.max, p.len, p.flags, p.console, p.cflags, p.in, p.out, p.err, p.dwFlags, p.show, p.envsize );
    str( h, "curdir", &p.curdir ); str( h, "dllpath", &p.dllpath ); str( h, "image", &p.image ); str( h, "cmdline", &p.cmdline );
    str( h, "title", &p.title ); str( h, "desktop", &p.desktop ); str( h, "shell", &p.shell ); printf( "runtime %u bytes\n", p.runtime.Length );
    env = calloc( p.envsize + 4, 1 );
    ReadProcessMemory( h, p.env, env, p.envsize, &n );
    for (e = env; *e && (char *)e < (char *)env + p.envsize; e += wcslen( e ) + 1) printf( "  env [%Iu] %ls\n", wcslen( e ), e );
    return 0;
}
