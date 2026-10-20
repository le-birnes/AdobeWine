/* service-elevate: what a service gets when it starts an elevated process for the user the way Adobe's
 * update service does: WTSQueryUserToken -> TokenLinkedToken -> DuplicateTokenEx(TokenPrimary)
 * -> CreateProcessAsUser. Prints each step; "service-elevate.exe child" prints the child's elevation.
 * Build: x86_64-w64-mingw32-gcc -O2 -o service-elevate.exe service-elevate.c -lwtsapi32 -ladvapi32 */
#include <windows.h>
#include <wtsapi32.h>
#include <stdio.h>
#include <string.h>

static void show( const char *what, HANDLE token )
{
    TOKEN_ELEVATION_TYPE type = 0;
    TOKEN_ELEVATION elevated = {0};
    TOKEN_TYPE ttype = 0;
    DWORD size;

    GetTokenInformation( token, TokenElevationType, &type, sizeof(type), &size );
    GetTokenInformation( token, TokenElevation, &elevated, sizeof(elevated), &size );
    GetTokenInformation( token, TokenType, &ttype, sizeof(ttype), &size );
    printf( "%s: elevation type %d (1 default, 2 full, 3 limited), elevated %lu, %s\n", what, type,
            elevated.TokenIsElevated, ttype == TokenPrimary ? "primary" : "impersonation" );
}

int main( int argc, char **argv )
{
    HANDLE user = 0, primary = 0;
    TOKEN_LINKED_TOKEN linked = {0};
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    char cmd[MAX_PATH + 16];
    DWORD size;

    if (argc > 1 && !strcmp( argv[1], "child" ))
    {
        show( "child process token", GetCurrentProcessToken() );
        return 0;
    }
    setvbuf( stdout, NULL, _IONBF, 0 );
    show( "own process token", GetCurrentProcessToken() );
    if (!WTSQueryUserToken( WTSGetActiveConsoleSessionId(), &user )) { printf( "WTSQueryUserToken failed %lu\n", GetLastError() ); return 1; }
    show( "WTSQueryUserToken", user );
    if (!GetTokenInformation( user, TokenLinkedToken, &linked, sizeof(linked), &size ))
    {
        printf( "TokenLinkedToken failed %lu\n", GetLastError() );
        return 1;
    }
    show( "linked token", linked.LinkedToken );
    if (!DuplicateTokenEx( linked.LinkedToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &primary ))
    {
        printf( "DuplicateTokenEx(TokenPrimary) failed %lu\n", GetLastError() );
        return 1;
    }
    show( "duplicated primary", primary );
    GetModuleFileNameA( NULL, cmd, MAX_PATH );
    strcat( cmd, " child" );
    if (!CreateProcessAsUserA( primary, NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ))
    {
        printf( "CreateProcessAsUser failed %lu\n", GetLastError() );
        return 1;
    }
    WaitForSingleObject( pi.hProcess, 10000 );
    return 0;
}
