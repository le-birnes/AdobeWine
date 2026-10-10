/* WinHttpReadData and automatic decompression (0039), synchronous session:
 *  - ReadData returns what is available (no more than QueryDataAvailable reported) instead of
 *    blocking until the buffer is full: /slow sends 100 of 4000 bytes, the rest 1.5 s later;
 *  - with WINHTTP_OPTION_DECOMPRESSION, Content-Length is gone from the header table and from
 *    the raw header text (Adobe GrowthSDK sized its buffer from the raw one, TESTLOG 2026-10-04);
 *  - a chunked keep-alive reply (plain and gzip) read with one large ReadData leaves the
 *    connection reusable: the next request on the same connect handle uses the same TCP
 *    connection (the first 0039 returned before the last chunk, so the connection was dropped).
 * Needs testserver.py on 127.0.0.1:18765. Prints one line per check, then PASS or FAIL. */
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string.h>

#ifndef WINHTTP_OPTION_DECOMPRESSION
#define WINHTTP_OPTION_DECOMPRESSION 118
#define WINHTTP_DECOMPRESSION_FLAG_ALL 3
#endif

static int failures;
static HINTERNET ses, con;

static void check(BOOL cond, const char *what)
{
    printf("%s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) failures++;
}

static HINTERNET get(const WCHAR *path)
{
    HINTERNET req = WinHttpOpenRequest(con, L"GET", path, NULL, NULL, NULL, 0);
    if (!req || !WinHttpSendRequest(req, NULL, 0, NULL, 0, 0, 0) || !WinHttpReceiveResponse(req, NULL))
    {
        printf("FAIL GET %ls: error %lu\n", path, GetLastError());
        failures++;
        if (req) WinHttpCloseHandle(req);
        return NULL;
    }
    return req;
}

static DWORD conn_id(HINTERNET req)
{
    WCHAR buf[16] = L"";
    DWORD size = sizeof(buf);
    WinHttpQueryHeaders(req, WINHTTP_QUERY_CUSTOM, L"X-Conn", buf, &size, NULL);
    return wcstoul(buf, NULL, 10);
}

static DWORD read_all(HINTERNET req, char *buf, DWORD size)
{
    DWORD got = 0, n;
    while (got < size && WinHttpReadData(req, buf + got, size - got, &n) && n) got += n;
    return got;
}

/* one ReadData with a large buffer, close the request, then check the next request reuses the connection */
static void reuse(const WCHAR *path, const char *expect, DWORD expect_len)
{
    static char buf[16384];
    char what[128];
    HINTERNET req, req2;
    DWORD n = 0, c1, c2;

    if (!(req = get(path))) return;
    c1 = conn_id(req);
    WinHttpReadData(req, buf, sizeof(buf), &n);
    snprintf(what, sizeof(what), "%ls: one ReadData returns the whole body (%lu of %lu bytes)", path, n, expect_len);
    check(n == expect_len && !memcmp(buf, expect, expect_len), what);
    WinHttpCloseHandle(req);
    if (!(req2 = get(L"/ok"))) return;
    c2 = conn_id(req2);
    read_all(req2, buf, sizeof(buf));
    WinHttpCloseHandle(req2);
    snprintf(what, sizeof(what), "%ls: next request reuses the keep-alive connection (conn %lu -> %lu)", path, c1, c2);
    check(c1 && c1 == c2, what);
}

int main(void)
{
    static char buf[16384], text[4096];
    DWORD flags = WINHTTP_DECOMPRESSION_FLAG_ALL, avail = 0, n = 0, len, size, t, i;
    WCHAR raw[4096], *p;
    BOOL found;
    HINTERNET req;

    for (i = len = 0; i < 60; i++) len += sprintf(text + len, "line %04lu of the decompressed body\n", i);

    ses = WinHttpOpen(L"readavail", WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
    check(WinHttpSetOption(ses, WINHTTP_OPTION_DECOMPRESSION, &flags, sizeof(flags)), "WINHTTP_OPTION_DECOMPRESSION");
    con = WinHttpConnect(ses, L"127.0.0.1", 18765, 0);

    /* (a) return what is available */
    if ((req = get(L"/slow")))
    {
        check(WinHttpQueryDataAvailable(req, &avail) && avail && avail < 4000, "/slow: QueryDataAvailable reports part of the body");
        t = GetTickCount();
        WinHttpReadData(req, buf, 4000, &n);
        t = GetTickCount() - t;
        printf("     /slow: available %lu, ReadData(4000) returned %lu in %lu ms\n", avail, n, t);
        check(n && n <= avail, "/slow: ReadData returns no more than QueryDataAvailable reported");
        check(t < 1000, "/slow: ReadData does not wait for the rest of the body");
        n += read_all(req, buf + n, sizeof(buf) - n);
        check(n == 4000 && buf[0] == '0' && buf[3999] == '9', "/slow: the rest of the body follows");
        WinHttpCloseHandle(req);
    }

    /* (b) no compressed Content-Length anywhere when decompressing */
    if ((req = get(L"/gzlen")))
    {
        size = sizeof(buf);
        check(!WinHttpQueryHeaders(req, WINHTTP_QUERY_CONTENT_LENGTH, NULL, buf, &size, NULL)
              && GetLastError() == ERROR_WINHTTP_HEADER_NOT_FOUND, "/gzlen: no Content-Length in the header table");
        size = sizeof(raw);
        check(WinHttpQueryHeaders(req, WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, raw, &size, NULL)
              && !wcsstr(raw, L"Content-Length") && wcsstr(raw, L"X-Conn"), "/gzlen: no Content-Length in the raw headers");
        size = sizeof(raw);
        memset(raw, 0, sizeof(raw));
        found = !WinHttpQueryHeaders(req, WINHTTP_QUERY_RAW_HEADERS, NULL, raw, &size, NULL);
        for (p = raw; *p; p += wcslen(p) + 1) if (!wcsnicmp(p, L"Content-Length", 14)) found = TRUE;
        check(!found, "/gzlen: no Content-Length in WINHTTP_QUERY_RAW_HEADERS");
        n = read_all(req, buf, sizeof(buf));
        check(n == len && !memcmp(buf, text, len), "/gzlen: decompressed body");
        WinHttpCloseHandle(req);
    }

    /* chunked keep-alive replies leave the connection reusable */
    reuse(L"/chunk", "123456789", 9);
    reuse(L"/gzchunk", text, len);

    WinHttpCloseHandle(con);
    WinHttpCloseHandle(ses);
    printf("%s: %d failures\n", failures ? "FAIL" : "PASS", failures);
    return !!failures;
}
