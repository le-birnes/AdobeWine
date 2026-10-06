/* Time WinHTTP proxy auto-detection the way UXP uses it: IE config, DetectAutoProxyConfigUrl,
 * GetProxyForUrl with WINHTTP_AUTOPROXY_AUTO_DETECT. */
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
static double now(void) { LARGE_INTEGER f, c; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&c); return (double)c.QuadPart * 1000 / f.QuadPart; }
int main(void)
{
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ie = {0};
    WINHTTP_AUTOPROXY_OPTIONS opt = {0};
    WINHTTP_PROXY_INFO info = {0};
    LPWSTR url = NULL;
    HINTERNET s;
    double t;
    BOOL r;

    t = now(); r = WinHttpGetIEProxyConfigForCurrentUser(&ie);
    printf("GetIEProxyConfig %d err %lu autodetect %d autoconfig %ls proxy %ls  %.1f ms\n", r, r ? 0 : GetLastError(),
           ie.fAutoDetect, ie.lpszAutoConfigUrl ? ie.lpszAutoConfigUrl : L"-", ie.lpszProxy ? ie.lpszProxy : L"-", now() - t);
    t = now(); r = WinHttpDetectAutoProxyConfigUrl(WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A, &url);
    printf("DetectAutoProxyConfigUrl %d err %lu url %ls  %.1f ms\n", r, r ? 0 : GetLastError(), url ? url : L"-", now() - t);
    s = WinHttpOpen(L"test", WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
    opt.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
    opt.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
    opt.fAutoLogonIfChallenged = TRUE;
    t = now(); r = WinHttpGetProxyForUrl(s, L"https://ims-na1.adobelogin.com/ims/token/v4", &opt, &info);
    printf("GetProxyForUrl %d err %lu  %.1f ms\n", r, r ? 0 : GetLastError(), now() - t);
    t = now(); r = WinHttpGetProxyForUrl(s, L"https://ims-na1.adobelogin.com/ims/token/v4", &opt, &info);
    printf("GetProxyForUrl again %d err %lu  %.1f ms\n", r, r ? 0 : GetLastError(), now() - t);
    return 0;
}
