/* DisplayConfigGetDeviceInfo(DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL) for every active
 * path, as Photoshop's GPU sniffer asks it after QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS |
 * QDC_VIRTUAL_MODE_AWARE). Windows answers with the SDR white level in units of 80 nits /
 * 1000 (1000 for a display without HDR); Wine failed with ERROR_INVALID_PARAMETER. */
#include <windows.h>
#include <stdio.h>
int main(void)
{
    DISPLAYCONFIG_PATH_INFO paths[16];
    DISPLAYCONFIG_MODE_INFO modes[48];
    UINT32 npaths = ARRAYSIZE(paths), nmodes = ARRAYSIZE(modes), i;
    LONG ret = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE, &npaths, paths, &nmodes, modes, NULL);
    int fails = 0;

    printf("QueryDisplayConfig %ld, %u paths\n", ret, npaths);
    if (ret || !npaths) { printf("FAIL\n"); return 1; }
    for (i = 0; i < npaths; i++)
    {
        DISPLAYCONFIG_SDR_WHITE_LEVEL white = {{DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL, sizeof(white),
                                               paths[i].targetInfo.adapterId, paths[i].targetInfo.id}};
        ret = DisplayConfigGetDeviceInfo(&white.header);
        printf("path %u target %u: ret %ld, SDRWhiteLevel %lu\n", i, paths[i].targetInfo.id, ret, white.SDRWhiteLevel);
        if (ret || white.SDRWhiteLevel < 1000) fails++;
    }
    printf("%s\n", fails ? "FAIL" : "PASS");
    return fails != 0;
}
