/* Does a freed block come straight back from the next allocation of the same size? */
#include <windows.h>
#include <stdio.h>

int main(void)
{
    HANDLE heap = GetProcessHeap();
    int i, same = 0;
    for (i = 0; i < 100; i++)
    {
        void *a = HeapAlloc(heap, 0, 200), *b;
        HeapFree(heap, 0, a);
        b = HeapAlloc(heap, 0, 200);
        if (a == b) same++;
        HeapFree(heap, 0, b);
    }
    printf("immediate reuse %d/100\n", same);
    return 0;
}
