/* winclose.exe <exe name>...: close apps like their window's close button does, by sending
 * SC_CLOSE to each visible, unowned top-level window of those processes. (taskkill posts
 * WM_CLOSE to every top-level window, hidden helpers too; Illustrator then exits without its
 * normal quit and leaves crash-recovery data behind.) Prints how many windows it closed. */
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

static DWORD pids[256];
static int npids, closed;

static BOOL CALLBACK close_window(HWND hwnd, LPARAM lparam)
{
    DWORD pid;
    int i;

    GetWindowThreadProcessId(hwnd, &pid);
    for (i = 0; i < npids; i++)
    {
        if (pids[i] != pid) continue;
        if (!IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER)) break;
        PostMessageW(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
        closed++;
        break;
    }
    return TRUE;
}

int main(int argc, char **argv)
{
    PROCESSENTRY32 pe = {sizeof(pe)};
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    int i;

    if (Process32First(snap, &pe))
    {
        do
            for (i = 1; i < argc; i++)
                if (!lstrcmpiA(pe.szExeFile, argv[i]) && npids < 256) pids[npids++] = pe.th32ProcessID;
        while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    EnumWindows(close_window, 0);
    printf("%d\n", closed);
    return 0;
}
