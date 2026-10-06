/* A serial work queue is unlocked while one of its items is still running, then MFShutdown.
 * The serial queue's finalization callback runs on the system pool after the item; Wine freed
 * (and zeroed) the serial queue at unlock, the callback blocked forever on the zeroed critical
 * section, and MFShutdown waited for that callback (Photoshop 2026 hung on exit this way). */
#define COBJMACROS
#include <windows.h>
#include <mfapi.h>
#include <stdio.h>

static HRESULT WINAPI qi(IMFAsyncCallback *iface, REFIID riid, void **obj)
{
    if (IsEqualIID(riid, &IID_IMFAsyncCallback) || IsEqualIID(riid, &IID_IUnknown)) { *obj = iface; return S_OK; }
    *obj = NULL; return E_NOINTERFACE;
}
static ULONG WINAPI addref(IMFAsyncCallback *iface) { return 2; }
static ULONG WINAPI release(IMFAsyncCallback *iface) { return 1; }
static HRESULT WINAPI getparams(IMFAsyncCallback *iface, DWORD *flags, DWORD *queue) { return E_NOTIMPL; }
static HRESULT WINAPI invoke(IMFAsyncCallback *iface, IMFAsyncResult *result) { Sleep(300); return S_OK; }
static IMFAsyncCallbackVtbl vtbl = { qi, addref, release, getparams, invoke };
static IMFAsyncCallback callback = { &vtbl };

static DWORD WINAPI watchdog(void *arg)
{
    Sleep(5000);
    printf("FAIL: MFShutdown still waiting after 5 s\n"); fflush(stdout);
    ExitProcess(1);
}

int main(void)
{
    DWORD serial, start;
    HRESULT hr;
    int i;

    MFStartup(MF_VERSION, MFSTARTUP_FULL);
    for (i = 0; i < 3; i++)
    {
        hr = MFAllocateSerialWorkQueue(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, &serial);
        if (FAILED(hr)) { printf("MFAllocateSerialWorkQueue %#lx\n", hr); return 1; }
        MFPutWorkItem(serial, &callback, NULL);
        Sleep(50);                 /* the item is running now */
        MFUnlockWorkQueue(serial); /* last reference: the queue is shut down */
    }
    CreateThread(NULL, 0, watchdog, NULL, 0, NULL);
    start = GetTickCount();
    hr = MFShutdown();
    printf("PASS: MFShutdown %#lx after %lu ms\n", hr, GetTickCount() - start);
    return 0;
}
