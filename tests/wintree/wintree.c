/* Print the window tree of a toplevel whose title contains argv[1] (or "@x,y"):
 * handle, class, client rect in screen coords, visible flag, styles. */
#include <windows.h>
#include <stdio.h>
#include <string.h>

int _dowildcard = 0;  /* mingw CRT: do not expand "*" as a file wildcard */
static const char *needle;
static HWND found;

static void dump(HWND hwnd, int depth)
{
    char cls[64], title[64];
    RECT r;
    HWND child;
    LONG style = GetWindowLongA(hwnd, GWL_STYLE), ex = GetWindowLongA(hwnd, GWL_EXSTYLE);

    GetClassNameA(hwnd, cls, sizeof(cls));
    GetWindowTextA(hwnd, title, sizeof(title));
    GetWindowRect(hwnd, &r);
    printf("%*s%p %-28s (%ld,%ld)-(%ld,%ld) %s%s%s ex %08lx \"%s\"\n", depth * 2, "", hwnd, cls,
           r.left, r.top, r.right, r.bottom, IsWindowVisible(hwnd) ? "vis" : "hid",
           style & WS_CLIPCHILDREN ? " clipch" : "", style & WS_CLIPSIBLINGS ? " clipsib" : "", ex, title);
    for (child = GetWindow(hwnd, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT))
        dump(child, depth + 1);
}

static BOOL CALLBACK find(HWND hwnd, LPARAM lp)
{
    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    if (IsWindowVisible(hwnd) && strstr(title, needle)) { found = hwnd; return FALSE; }
    return TRUE;
}

int main(int argc, char **argv)
{
    needle = argc > 1 ? argv[1] : "Adobe";
    if (!strcmp(needle, "*") || !strcmp(needle, "all"))  /* toplevels, top of the z-order first;
                                                            "all" adds hidden ones (marked hid) */
    {
        BOOL hidden = !strcmp(needle, "all");
        HWND w;
        for (w = GetTopWindow(0); w; w = GetWindow(w, GW_HWNDNEXT))
        {
            char cls[64], title[64]; RECT r; DWORD pid, flags = 0; BYTE alpha = 0; COLORREF key = 0;
            if (!IsWindowVisible(w) && !hidden) continue;
            if (hidden) printf("%s ", IsWindowVisible(w) ? "vis" : "hid");
            GetClassNameA(w, cls, sizeof(cls)); GetWindowTextA(w, title, sizeof(title));
            GetWindowRect(w, &r); GetWindowThreadProcessId(w, &pid);
            if (GetWindowLongA(w, GWL_EXSTYLE) & WS_EX_LAYERED) GetLayeredWindowAttributes(w, &key, &alpha, &flags);
            printf("%p pid %04lx owner %p style %08lx ex %08lx %-24s (%ld,%ld)-(%ld,%ld) lwa %lx/%u key %06lx \"%s\"\n", w, pid,
                   GetWindow(w, GW_OWNER), GetWindowLongA(w, GWL_STYLE), GetWindowLongA(w, GWL_EXSTYLE),
                   cls, r.left, r.top, r.right, r.bottom, flags, alpha, key, title);
        }
        return 0;
    }
    if (needle[0] == '@')  /* "@x,y": the toplevel under that screen point */
    {
        POINT pt;
        sscanf(needle + 1, "%ld,%ld", &pt.x, &pt.y);
        found = GetAncestor(WindowFromPoint(pt), GA_ROOT);
    }
    else EnumWindows(find, 0);
    if (!found) { printf("no window with \"%s\"\n", needle); return 1; }
    dump(found, 0);
    return 0;
}
