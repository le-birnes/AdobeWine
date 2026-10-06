/* Async WinHTTP: WinHttpReceiveResponse called from the SENDREQUEST_COMPLETE callback must
 * return at once (completion arrives later as HEADERS_AVAILABLE), as on Windows. Wine read
 * the reply inside the call, so an app holding a lock there (Photoshop) waited for the server.
 * Needs slowserver.py (answers after 3 s) on 127.0.0.1:18765. */
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>

static HANDLE done;
static DWORD call_ms = 0xffffffff;

static void CALLBACK cb(HINTERNET h, DWORD_PTR ctx, DWORD status, void *info, DWORD len)
{
    if (status == WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE)
    {
        DWORD t = GetTickCount();
        WinHttpReceiveResponse(h, NULL);
        call_ms = GetTickCount() - t;
    }
    else if (status == WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE || status == WINHTTP_CALLBACK_STATUS_REQUEST_ERROR)
        SetEvent(done);
}

int main(void)
{
    HINTERNET s, c, r;
    DWORD code = 0, size = sizeof(code);
    done = CreateEventA(NULL, TRUE, FALSE, NULL);
    s = WinHttpOpen(L"t", WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, WINHTTP_FLAG_ASYNC);
    WinHttpSetStatusCallback(s, cb, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
    c = WinHttpConnect(s, L"127.0.0.1", 18765, 0);
    r = WinHttpOpenRequest(c, L"POST", L"/ingest", NULL, NULL, NULL, 0);
    WinHttpSendRequest(r, NULL, 0, (void *)"x", 1, 1, 0);
    WaitForSingleObject(done, 15000);
    WinHttpQueryHeaders(r, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &code, &size, NULL);
    printf("status %lu, WinHttpReceiveResponse in callback took %lu ms: %s\n", code, call_ms, call_ms < 500 ? "PASS" : "FAIL");
    return call_ms >= 500;
}
