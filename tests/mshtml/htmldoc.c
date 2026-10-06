/* CoCreateInstance(CLSID_HTMLDocument) as Adobe Desktop Service does it (32-bit, STA and MTA). */
#include <windows.h>
#include <stdio.h>
static const GUID CLSID_HTMLDocument_ = {0x25336920,0x03f9,0x11cf,{0x8f,0xd0,0x00,0xaa,0x00,0x68,0x6f,0x13}};
static const GUID IID_IUnknown_ = {0,0,0,{0xc0,0,0,0,0,0,0,0x46}};
int main(int argc, char **argv)
{
    IUnknown *unk = NULL;
    HRESULT hr = CoInitializeEx(NULL, argc > 1 ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED);
    printf("CoInitializeEx %#lx (%s)\n", hr, argc > 1 ? "MTA" : "STA");
    hr = CoCreateInstance(&CLSID_HTMLDocument_, NULL, CLSCTX_INPROC_SERVER, &IID_IUnknown_, (void **)&unk);
    printf("CoCreateInstance(HTMLDocument) %#lx %p\n", hr, unk);
    return 0;
}
