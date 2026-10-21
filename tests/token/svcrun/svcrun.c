/* svcrun: a service that starts the command line in HKLM\Software\svcrun\Cmd with the user's linked
 * (elevated) token, like Adobe's update service, and logs to C:\svcrun.log. Optional values: Desktop
 * (STARTUPINFO lpDesktop), Plain (any value: CreateProcess with the service's own token and window station). */
#include <windows.h>
#include <wtsapi32.h>
#include <stdio.h>
static SERVICE_STATUS_HANDLE h;
static SERVICE_STATUS st = { SERVICE_WIN32_OWN_PROCESS, SERVICE_RUNNING, 0 };
static void WINAPI ctrl( DWORD c ) { }
static void WINAPI svcmain( DWORD argc, LPSTR *argv )
{
    FILE *f = fopen( "C:\\svcrun.log", "w" );
    HANDLE user = 0, primary = 0; TOKEN_LINKED_TOKEN linked = {0}; DWORD size = sizeof(linked);
    char cmd[2048]; DWORD len = sizeof(cmd); STARTUPINFOA si = { sizeof(si) }; PROCESS_INFORMATION pi; DWORD code = 0;
    h = RegisterServiceCtrlHandlerA( "svcrun", ctrl ); SetServiceStatus( h, &st );
    RegGetValueA( HKEY_LOCAL_MACHINE, "Software\\svcrun", "Cmd", RRF_RT_REG_SZ, NULL, cmd, &len );
    char desk[256]; DWORD dlen = sizeof(desk), plain = 0, plen = sizeof(plain);
    if (!RegGetValueA( HKEY_LOCAL_MACHINE, "Software\\svcrun", "Desktop", RRF_RT_REG_SZ, NULL, desk, &dlen )) si.lpDesktop = desk;
    RegGetValueA( HKEY_LOCAL_MACHINE, "Software\\svcrun", "Plain", RRF_RT_REG_DWORD, NULL, &plain, &plen );
    fprintf( f, "cmd %s desktop %s plain %lu\n", cmd, si.lpDesktop ? si.lpDesktop : "(null)", plain );
    if (plain)
    {
        if (!CreateProcessA( NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi )) fprintf( f, "create %lu\n", GetLastError() );
        else { WaitForSingleObject( pi.hProcess, 120000 ); GetExitCodeProcess( pi.hProcess, &code ); fprintf( f, "exit %#lx\n", code ); }
    }
    else
    if (!WTSQueryUserToken( WTSGetActiveConsoleSessionId(), &user )) fprintf( f, "WTSQueryUserToken %lu\n", GetLastError() );
    else if (!GetTokenInformation( user, TokenLinkedToken, &linked, sizeof(linked), &size )) fprintf( f, "linked %lu\n", GetLastError() );
    else if (!DuplicateTokenEx( linked.LinkedToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &primary )) fprintf( f, "dup %lu\n", GetLastError() );
    else if (!CreateProcessAsUserA( primary, NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi )) fprintf( f, "create %lu\n", GetLastError() );
    else { WaitForSingleObject( pi.hProcess, 120000 ); GetExitCodeProcess( pi.hProcess, &code ); fprintf( f, "exit %#lx\n", code ); }
    fclose( f );
    st.dwCurrentState = SERVICE_STOPPED; SetServiceStatus( h, &st );
}
int main(void)
{
    SERVICE_TABLE_ENTRYA t[] = { { "svcrun", svcmain }, { 0 } };
    return !StartServiceCtrlDispatcherA( t );
}
