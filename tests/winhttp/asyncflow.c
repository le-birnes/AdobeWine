/* WinHTTP request flows that the queued async receive must keep working:
 * async GET with a redirect (302 -> 200) and the body read through callbacks,
 * and the same GET with a synchronous session. Needs testserver.py on 127.0.0.1:18765. */
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string.h>

static HANDLE done;
static char body[64];
static DWORD got, status_code, redirects;

static void CALLBACK cb(HINTERNET h, DWORD_PTR ctx, DWORD status, void *info, DWORD len)
{
    DWORD size = sizeof(status_code);
    switch (status)
    {
    case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: WinHttpReceiveResponse(h, NULL); break;
    case WINHTTP_CALLBACK_STATUS_REDIRECT: redirects++; break;
    case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
        WinHttpQueryHeaders(h, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &status_code, &size, NULL);
        WinHttpReadData(h, body + got, sizeof(body) - 1 - got, NULL);
        break;
    case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
        if (!len) { SetEvent(done); break; }
        got += len;
        WinHttpReadData(h, body + got, sizeof(body) - 1 - got, NULL);
        break;
    case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: SetEvent(done); break;
    }
}

int main(void)
{
    HINTERNET s, c, r;
    DWORD code = 0, size = sizeof(code), n = 0;
    int fail = 0;

    done = CreateEventA(NULL, TRUE, FALSE, NULL);
    s = WinHttpOpen(L"t", WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, WINHTTP_FLAG_ASYNC);
    WinHttpSetStatusCallback(s, cb, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
    c = WinHttpConnect(s, L"127.0.0.1", 18765, 0);
    r = WinHttpOpenRequest(c, L"GET", L"/r", NULL, NULL, NULL, 0);
    WinHttpSendRequest(r, NULL, 0, NULL, 0, 0, 0);
    WaitForSingleObject(done, 10000);
    printf("async: status %lu, redirects %lu, body '%s'\n", status_code, redirects, body);
    if (status_code != 200 || redirects != 1 || strcmp(body, "hello")) fail = 1;
    WinHttpCloseHandle(r); WinHttpCloseHandle(c); WinHttpCloseHandle(s);

    memset(body, 0, sizeof(body));
    s = WinHttpOpen(L"t", WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
    c = WinHttpConnect(s, L"127.0.0.1", 18765, 0);
    r = WinHttpOpenRequest(c, L"GET", L"/r", NULL, NULL, NULL, 0);
    if (!WinHttpSendRequest(r, NULL, 0, NULL, 0, 0, 0) || !WinHttpReceiveResponse(r, NULL)) fail = 1;
    WinHttpQueryHeaders(r, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &code, &size, NULL);
    WinHttpReadData(r, body, sizeof(body) - 1, &n);
    printf("sync: status %lu, body '%s'\n", code, body);
    if (code != 200 || strcmp(body, "hello")) fail = 1;
    printf("%s\n", fail ? "FAIL" : "PASS");
    return fail;
}
