/* Print this process's integrity level; with "spawn", start a copy with a medium-integrity token. */
#include <windows.h>
#include <stdio.h>
static DWORD level(HANDLE token)
{
    BYTE buf[64]; DWORD len; TOKEN_MANDATORY_LABEL *tml = (void *)buf;
    DWORD i;
    memset(buf, 0xcc, sizeof(buf));
    if (!GetTokenInformation(token, TokenIntegrityLevel, buf, sizeof(buf), &len)) return GetLastError() | 0x80000000;
    if (getenv("DUMP")) { printf("len %lu:", len); for (i = 0; i < len; i++) printf(" %02x", buf[i]); printf(" (buf %p sid %p)\n", buf, tml->Label.Sid); }
    return *GetSidSubAuthority(tml->Label.Sid, *GetSidSubAuthorityCount(tml->Label.Sid) - 1);
}
int main(int argc, char **argv)
{
    HANDLE token, dup; STARTUPINFOW si = { sizeof(si) }; PROCESS_INFORMATION pi;
    SID_IDENTIFIER_AUTHORITY ml = SECURITY_MANDATORY_LABEL_AUTHORITY; PSID medium;
    TOKEN_MANDATORY_LABEL tml; WCHAR cmd[MAX_PATH + 16];
    OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &token);
    printf("%s: integrity %#lx\n", argc > 1 && !strcmp(argv[1], "child") ? "child" : "parent", level(token));
    if (argc < 2 || strcmp(argv[1], "spawn")) return 0;
    DuplicateTokenEx(token, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &dup);
    AllocateAndInitializeSid(&ml, 1, SECURITY_MANDATORY_MEDIUM_RID, 0, 0, 0, 0, 0, 0, 0, &medium);
    tml.Label.Sid = medium; tml.Label.Attributes = SE_GROUP_INTEGRITY;
    printf("set: %d (error %lu), dup now %#lx, original still %#lx\n",
           SetTokenInformation(dup, TokenIntegrityLevel, &tml, sizeof(tml) + GetLengthSid(medium)), GetLastError(),
           level(dup), level(token));
    GetModuleFileNameW(NULL, cmd, MAX_PATH); wcscat(cmd, L" child");
    if (CreateProcessAsUserW(dup, NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        WaitForSingleObject(pi.hProcess, 10000);
    else printf("CreateProcessAsUser failed %lu\n", GetLastError());
    return 0;
}
