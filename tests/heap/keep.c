/* 0027: an emptied subheap stays mapped, so an app that touches or frees a freed block again
 * (use after free, double free) does not fault.
 *
 * A growable heap gets a block too big for its first subheap, so the block lands in a subheap
 * of its own. Freeing it empties that subheap: stock Wine releases it (the block's memory is
 * gone, touching it faults), 0027 keeps the range mapped and the block header readable. Then
 * the freed block is read and written and freed a second time (must fail cleanly, not crash),
 * and the heap must still validate and allocate.
 * Built for both arches by run-headless.sh (keep.exe, keep32.exe). */
#include <windows.h>
#include <stdio.h>

int main(void)
{
    MEMORY_BASIC_INFORMATION before, after;
    HANDLE heap = HeapCreate( 0, 0, 0 );
    SIZE_T size = 0x40000;  /* above the first subheap, below the large block threshold */
    unsigned char *ptr, *a, *b;
    volatile unsigned char *v;
    int i, sum = 0;

    if (!heap) { printf( "FAIL HeapCreate %lu\n", GetLastError() ); return 1; }
    if (!(ptr = HeapAlloc( heap, 0, size ))) { printf( "FAIL HeapAlloc %lu\n", GetLastError() ); return 1; }
    memset( ptr, 0x5a, size );
    VirtualQuery( ptr, &before, sizeof(before) );
    if (!HeapFree( heap, 0, ptr )) { printf( "FAIL HeapFree %lu\n", GetLastError() ); return 1; }

    VirtualQuery( ptr, &after, sizeof(after) );
    printf( "block %p: region %p before free, after free state %#lx base %p\n", ptr,
            before.AllocationBase, after.State, after.AllocationBase );
    if (after.State != MEM_COMMIT || after.AllocationBase != before.AllocationBase)
    {
        printf( "FAIL freed block's subheap was released (state %#lx), touching it would fault\n", after.State );
        return 1;
    }

    /* use after free: read and write the start of the freed block */
    v = ptr;
    for (i = 0; i < 256; i++) sum += v[i];
    for (i = 64; i < 256; i++) v[i] = 0xcc;

    /* double free: caught as an invalid block, no crash */
    if (HeapFree( heap, 0, ptr )) { printf( "FAIL second HeapFree of %p succeeded\n", ptr ); return 1; }
    if (!HeapValidate( heap, 0, NULL )) { printf( "FAIL HeapValidate\n" ); return 1; }

    /* the heap keeps working; a new allocation of the same size gets a fresh subheap */
    a = HeapAlloc( heap, 0, size );
    b = HeapAlloc( heap, 0, 16 );
    if (!a || !b) { printf( "FAIL allocation after the double free\n" ); return 1; }
    memset( a, 1, size );
    HeapFree( heap, 0, b );
    HeapFree( heap, 0, a );
    if (!HeapValidate( heap, 0, NULL )) { printf( "FAIL HeapValidate at the end\n" ); return 1; }
    if (!HeapDestroy( heap )) { printf( "FAIL HeapDestroy %lu\n", GetLastError() ); return 1; }

    printf( "PASS freed block in an emptied subheap stays mapped (sum %d), double free rejected\n", sum );
    return 0;
}
