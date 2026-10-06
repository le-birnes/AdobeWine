/* Read all video samples of a file through Media Foundation's source reader. */
#define COBJMACROS
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
int wmain(int argc, WCHAR **argv)
{
    IMFSourceReader *r; IMFSample *s; HRESULT hr; DWORD idx, flags; LONGLONG ts; int n = 0;
    CoInitializeEx(NULL, COINIT_MULTITHREADED); MFStartup(MF_VERSION, 0);
    hr = MFCreateSourceReaderFromURL(argv[1], NULL, &r);
    printf("create %#lx\n", hr); if (FAILED(hr)) return 1;
    for (;;) {
        hr = IMFSourceReader_ReadSample(r, MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &idx, &flags, &ts, &s);
        if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) break;
        if (s) { n++; IMFSample_Release(s); }
    }
    printf("read %#lx samples %d last ts %.2f s\n", hr, n, ts / 1e7);
    return 0;
}
