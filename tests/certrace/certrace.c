/* certrace: threads enumerate a memory store while others add and delete in it, as Acrobat's signature checks do.
 * crypt32 took the enumerator's reference outside the store's lock, so a context could be freed with a reference
 * on it: assert(!context->ref) in Context_Free aborted acrotray and AdobeCollabSync. Exits 0 when it survives. */
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>

static HCERTSTORE store;
static const BYTE *encoded;
static DWORD encoded_size;
static volatile LONG stop;

static DWORD WINAPI enumerator(void *arg)
{
    while (!stop)
    {
        PCCERT_CONTEXT c = NULL;
        while ((c = CertEnumCertificatesInStore(store, c))) ;
    }
    return 0;
}

static DWORD WINAPI churn(void *arg)
{
    while (!stop)
    {
        PCCERT_CONTEXT c = NULL;
        if (CertAddEncodedCertificateToStore(store, X509_ASN_ENCODING, encoded, encoded_size,
                                             CERT_STORE_ADD_ALWAYS, &c))
            CertDeleteCertificateFromStore(c);
    }
    return 0;
}

int main(void)
{
    HANDLE t[8];
    HCERTSTORE root;
    PCCERT_CONTEXT c;
    int i;

    if (!(root = CertOpenSystemStoreA(0, "ROOT")) || !(c = CertEnumCertificatesInStore(root, NULL)))
    {
        printf("SKIP no certificate to copy\n");
        return 0;
    }
    encoded = c->pbCertEncoded;
    encoded_size = c->cbCertEncoded;
    store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, NULL);
    for (i = 0; i < 8; i++) t[i] = CreateThread(NULL, 0, i & 1 ? churn : enumerator, NULL, 0, NULL);
    Sleep(15000);
    stop = 1;
    WaitForMultipleObjects(8, t, TRUE, INFINITE);
    CertCloseStore(store, 0);
    printf("PASS survived\n");
    return 0;
}
