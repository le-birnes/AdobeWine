/* GetTokenInformation size queries for fixed-size UAC classes: Windows fails with ERROR_BAD_LENGTH. */
#include <windows.h>
#include <stdio.h>
int main(void)
{
    HANDLE token; DWORD len, i; static const int classes[] = { TokenLinkedToken, TokenElevationType, TokenElevation, TokenUser };
    OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token);
    for (i = 0; i < 4; i++)
    {
        BOOL r; SetLastError(0xdeadbeef);
        r = GetTokenInformation(token, classes[i], NULL, 0, &len);
        printf("class %d: ret %d error %lu len %lu\n", classes[i], r, GetLastError(), len);
    }
    return 0;
}
