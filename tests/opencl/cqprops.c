/* clCreateCommandQueueWithProperties (OpenCL 2.0) through Wine's OpenCL.dll, which exports the 1.2 API.
 * Premiere's OpenVINO GPU plugin imports it; without it Premiere quits after the splash on AMD (issue #15).
 * Creates a queue with and without properties, runs a fill and reads it back.
 * Build: x86_64-w64-mingw32-gcc -O2 -o cqprops.exe cqprops.c */
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

typedef void *cl_ptr;
typedef int32_t cl_int;
typedef uint64_t cl_ulong;
typedef cl_int (WINAPI *get_platforms_fn)(uint32_t, cl_ptr *, uint32_t *);
typedef cl_int (WINAPI *get_devices_fn)(cl_ptr, cl_ulong, uint32_t, cl_ptr *, uint32_t *);
typedef cl_ptr (WINAPI *create_context_fn)(const intptr_t *, uint32_t, const cl_ptr *, void *, void *, cl_int *);
typedef cl_ptr (WINAPI *create_queue_props_fn)(cl_ptr, cl_ptr, const cl_ulong *, cl_int *);
typedef cl_ptr (WINAPI *create_buffer_fn)(cl_ptr, cl_ulong, size_t, void *, cl_int *);
typedef cl_int (WINAPI *read_buffer_fn)(cl_ptr, cl_ptr, uint32_t, size_t, size_t, void *, uint32_t, const cl_ptr *, cl_ptr *);
typedef cl_int (WINAPI *write_buffer_fn)(cl_ptr, cl_ptr, uint32_t, size_t, size_t, const void *, uint32_t, const cl_ptr *, cl_ptr *);
typedef cl_int (WINAPI *release_fn)(cl_ptr);

int main(void)
{
    HMODULE cl = LoadLibraryA("OpenCL.dll");
    get_platforms_fn get_platforms = (void *)GetProcAddress(cl, "clGetPlatformIDs");
    get_devices_fn get_devices = (void *)GetProcAddress(cl, "clGetDeviceIDs");
    create_context_fn create_context = (void *)GetProcAddress(cl, "clCreateContext");
    create_queue_props_fn create_queue = (void *)GetProcAddress(cl, "clCreateCommandQueueWithProperties");
    create_buffer_fn create_buffer = (void *)GetProcAddress(cl, "clCreateBuffer");
    write_buffer_fn write_buffer = (void *)GetProcAddress(cl, "clEnqueueWriteBuffer");
    read_buffer_fn read_buffer = (void *)GetProcAddress(cl, "clEnqueueReadBuffer");
    release_fn release_queue = (void *)GetProcAddress(cl, "clReleaseCommandQueue");
    cl_ulong props[] = { 0x1093 /* CL_QUEUE_PROPERTIES */, 2 /* PROFILING */, 0 }, bad[] = { 0x1234, 1, 0 };
    cl_ptr platform, device, context, queue, buffer;
    unsigned int in[256], out[256], i, failures = 0;
    cl_int err;

    if (!create_queue) { printf("FAIL clCreateCommandQueueWithProperties not exported\n"); return 1; }
    if (get_platforms(1, &platform, NULL) || get_devices(platform, 0xffffffff, 1, &device, NULL))
    { printf("SKIP no OpenCL device\n"); return 0; }
    context = create_context(NULL, 1, &device, NULL, NULL, &err);
    if (!context) { printf("FAIL clCreateContext %d\n", err); return 1; }

    queue = create_queue(context, device, NULL, &err);
    printf("%s no properties: err %d\n", queue ? "ok" : "FAIL", err); failures += !queue;
    if (queue) release_queue(queue);
    queue = create_queue(context, device, bad, &err);
    printf("%s unknown property rejected: err %d\n", !queue && err == -30 ? "ok" : "FAIL", err); failures += !!queue;
    queue = create_queue(context, device, props, &err);
    printf("%s profiling queue: err %d\n", queue ? "ok" : "FAIL", err); failures += !queue;
    if (!queue) return 1;

    for (i = 0; i < 256; i++) in[i] = i * 2654435761u;
    buffer = create_buffer(context, 1 /* READ_WRITE */, sizeof(in), NULL, &err);
    err = write_buffer(queue, buffer, 1, 0, sizeof(in), in, 0, NULL, NULL);
    err |= read_buffer(queue, buffer, 1, 0, sizeof(out), out, 0, NULL, NULL);
    i = !err && !memcmp(in, out, sizeof(in));
    printf("%s write/read through the queue\n", i ? "ok" : "FAIL"); failures += !i;
    printf("result: %s\n", failures ? "FAIL" : "ok");
    return failures != 0;
}
