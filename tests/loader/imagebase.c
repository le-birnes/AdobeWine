/* A thread started after the program changed PEB->ImageBaseAddress (self-unpacking or self-mapping installers do
 * that; public #18, Adobe's Creative_Cloud_Set-Up.exe on an Intel laptop) crashed in ntdll's loader_init:
 * a new thread looks the main module up by that address (get_modref( Peb->ImageBaseAddress )), gets NULL and
 * reads its TlsIndex (c0000005 at loader_init+0x2f4 in 0.4.0's 32-bit ntdll). Windows keeps the main module's
 * entry itself, so the thread just starts.
 *   imagebase.exe    prints "thread ran" and PASS, or crashes (FAIL) on an unpatched Wine
 * Build: i686-w64-mingw32-gcc -O2 -o imagebase32.exe imagebase.c   (and x86_64-w64-mingw32-gcc for 64-bit) */
#include <windows.h>
#include <winternl.h>
#include <stdio.h>

static DWORD WINAPI thread_proc(void *arg)
{
    *(volatile LONG *)arg = 1;
    return 0;
}

int main(void)
{
    PEB *peb = NtCurrentTeb()->ProcessEnvironmentBlock;
    void *saved = peb->Reserved3[1];  /* ImageBaseAddress */
    void *other = VirtualAlloc(NULL, 0x10000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    volatile LONG ran = 0;
    HANDLE thread;
    DWORD wait;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (saved != GetModuleHandleW(NULL)) { printf("unexpected PEB layout\nFAIL\n"); return 1; }
    peb->Reserved3[1] = other;        /* what an in-process unpacker does after mapping its payload */
    thread = CreateThread(NULL, 0, thread_proc, (void *)&ran, 0, NULL);
    wait = WaitForSingleObject(thread, 10000);
    peb->Reserved3[1] = saved;
    printf("%s\n", ran ? "thread ran" : "thread did not run");
    printf("%s\n", wait == WAIT_OBJECT_0 && ran ? "PASS" : "FAIL");
    return !(wait == WAIT_OBJECT_0 && ran);
}
