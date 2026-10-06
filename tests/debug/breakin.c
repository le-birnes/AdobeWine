/* 64-bit process calls DebugBreakProcess on a 32-bit (wow64) child that is not being debugged.
 * The break-in thread must run the child's DbgUiRemoteBreakin, which just exits when no
 * debugger is attached; the child keeps running and exits with its own code (7).
 * Wine started that thread at the 64-bit ntdll address cut to 32 bits and the child crashed. */
#include <windows.h>
#include <stdio.h>
int main(void)
{
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    char cmd[] = "sleeper32.exe";
    DWORD code;
    BOOL wow = FALSE;

    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) { printf("no sleeper32.exe\n"); return 1; }
    IsWow64Process(pi.hProcess, &wow);
    Sleep(1000);
    printf("child wow64 %d, DebugBreakProcess %d\n", wow, DebugBreakProcess(pi.hProcess));
    WaitForSingleObject(pi.hProcess, 15000);
    GetExitCodeProcess(pi.hProcess, &code);
    printf("child exit code %lu: %s\n", code, code == 7 ? "PASS" : "FAIL");
    return code != 7;
}
