// wv2msg.exe [hidden]: WebView2 host<->page messaging test. The page posts "ping N" five times,
// one per second; the host answers each with "pong N"; the page posts "got pong N" when it
// receives one. Prints every message the host receives. With "hidden", the WebView's parent
// is a hidden 0x0 window (as Illustrator's preloaded Home WebView is).
#include <windows.h>
#include <stdio.h>
#include "WebView2.h"

static ICoreWebView2 *webview;
static ICoreWebView2Controller *controller;
static HWND hwnd;
static bool hidden_mode, delayed, invisible;
static int pending, sent;
static LRESULT CALLBACK wndproc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_TIMER && pending) {
        KillTimer(h, 1); pending--; wchar_t r[64]; swprintf(r, 64, L"late pong %d", ++sent);
        HRESULT hr = webview->PostWebMessageAsString(r);
        printf("[%lu]  timer posted %ls hr %08lx\n", GetTickCount() % 100000, r, hr); fflush(stdout);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

template <class I> struct Handler : I {
    ULONG ref = 1;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **o) override { *o = this; AddRef(); return S_OK; }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++ref; }
    ULONG STDMETHODCALLTYPE Release() override { return --ref; }
};

struct MsgHandler : Handler<ICoreWebView2WebMessageReceivedEventHandler> {
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2 *s, ICoreWebView2WebMessageReceivedEventArgs *a) override {
        LPWSTR m = nullptr;
        a->TryGetWebMessageAsString(&m);
        printf("[%lu] host got: %ls\n", GetTickCount() % 100000, m ? m : L"(null)"); fflush(stdout);
        if (m && !wcsncmp(m, L"ping", 4)) {
            if (delayed) { pending++; SetTimer(hwnd, 1, 1500, NULL); }
            else {
                wchar_t r[64]; swprintf(r, 64, L"pong%ls", m + 4);
                HRESULT hr = s->PostWebMessageAsString(r);
                printf("  posted %ls hr %08lx\n", r, hr); fflush(stdout);
            }
        }
        CoTaskMemFree(m);
        return S_OK;
    }
};

struct NavHandler : Handler<ICoreWebView2NavigationCompletedEventHandler> {
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2 *, ICoreWebView2NavigationCompletedEventArgs *a) override {
        BOOL ok = 0; a->get_IsSuccess(&ok);
        printf("[%lu] navigation completed ok %d\n", GetTickCount() % 100000, ok); fflush(stdout);
        return S_OK;
    }
};

static const wchar_t page[] =
    L"<html><body><script>"
    L"let n=0;window.chrome.webview.addEventListener('message',e=>window.chrome.webview.postMessage('got '+e.data));"
    L"const t=setInterval(()=>{n++;window.chrome.webview.postMessage('ping '+n);if(n>=5)clearInterval(t);},1000);"
    L"</script>hello</body></html>";

struct ControllerHandler : Handler<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler> {
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT hr, ICoreWebView2Controller *c) override {
        printf("controller hr %08lx\n", hr); fflush(stdout);
        if (FAILED(hr)) { PostQuitMessage(1); return S_OK; }
        controller = c; c->AddRef();
        c->get_CoreWebView2(&webview);
        RECT r; GetClientRect(hwnd, &r); if (!hidden_mode) c->put_Bounds(r);
        if (invisible) { RECT z = {0, 0, 0, 0}; c->put_Bounds(z); HRESULT h2 = c->put_IsVisible(FALSE); printf("put_IsVisible(FALSE) %08lx\n", h2); }
        EventRegistrationToken tok;
        webview->add_WebMessageReceived(new MsgHandler, &tok);
        webview->add_NavigationCompleted(new NavHandler, &tok);
        webview->NavigateToString(page);
        return S_OK;
    }
};

struct EnvHandler : Handler<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler> {
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT hr, ICoreWebView2Environment *env) override {
        printf("environment hr %08lx\n", hr); fflush(stdout);
        if (FAILED(hr)) { PostQuitMessage(1); return S_OK; }
        env->CreateCoreWebView2Controller(hwnd, new ControllerHandler);
        return S_OK;
    }
};

typedef HRESULT (WINAPI *create_env_fn)(PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions *, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *);

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) { if (!strcmp(argv[i], "hidden")) hidden_mode = true; if (!strcmp(argv[i], "delayed")) delayed = true; if (!strcmp(argv[i], "invisible")) invisible = true; }
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    WNDCLASSW wc = {}; wc.lpfnWndProc = wndproc; wc.hInstance = GetModuleHandleW(0); wc.lpszClassName = L"wv2msg";
    RegisterClassW(&wc);
    if (hidden_mode)
        hwnd = CreateWindowExW(0, L"wv2msg", L"wv2msg", WS_POPUP, 0, 0, 0, 0, 0, 0, wc.hInstance, 0);
    else
        hwnd = CreateWindowExW(0, L"wv2msg", L"wv2msg", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 640, 480, 0, 0, wc.hInstance, 0);
    HMODULE l = LoadLibraryW(L"WebView2Loader.dll");
    create_env_fn create = (create_env_fn)GetProcAddress(l, "CreateCoreWebView2EnvironmentWithOptions");
    wchar_t tmp[MAX_PATH]; GetTempPathW(MAX_PATH, tmp); wcscat(tmp, L"wv2msg");
    HRESULT hr = create(L"C:\\Program Files\\Common Files\\Adobe\\Microsoft\\EdgeWebView", tmp, NULL, new EnvHandler);
    printf("create env %08lx\n", hr); fflush(stdout);
    DWORD end = GetTickCount() + 25000;
    MSG msg;
    while (GetTickCount() < end) {
        if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
        else MsgWaitForMultipleObjects(0, 0, FALSE, 50, QS_ALLINPUT);
    }
    printf("done\n");
    return 0;
}
