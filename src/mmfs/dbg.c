#include "dbg.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

int debug = 0;
int mallocs = 0, frees = 0;

void dbgLog(const char *fmt, ...) {
    if (!debug) return;
    va_list args;
    printf("[DEBUG] ");
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void dbgSetDebug(int val) {
    debug = val;
}

void *dbgMalloc(size_t size, char *sourceFunc) {
    void *ptr = malloc(size);
    dbgLog("dbgMalloc from %s: size=%zu ptr=%zu\n", sourceFunc, size, ptr);
    if (ptr != NULL) mallocs++;
    return ptr;
}
void dbgFree(void *ptr, char *sourceFunc) {
    dbgLog("dbgFree from %s: ptr=%zu\n", sourceFunc, ptr);
    free(ptr);
    if (ptr != NULL) frees++;
}

size_t dbgFread(void *buf, size_t size, size_t n, FILE *f, char *sourceFunc) {
    size_t before = ftell(f);
    size_t result = fread(buf, size, n, f);
    size_t after = ftell(f);
    dbgLog("dbgFread from %s: size=%zu n=%zu result=%zu | before read @ %zu | after read @ %zu\n", sourceFunc, size, n, result, before, after);
    return result;
}

size_t dbgFwrite(void *buf, size_t size, size_t n, FILE *f, char *sourceFunc) {
    size_t before = ftell(f);
    size_t result = fwrite(buf, size, n, f);
    size_t after = ftell(f);
    dbgLog("dbgFwrite from %s: size=%zu n=%zu result=%zu | before write @ %zu | after write @ %zu\n", sourceFunc, size, n, result, before, after);
    return result;
}

void dbgPrintMemStats() {
    printf("=== MEMORY STATS - mallocs: %d - frees: %d ===\n", mallocs, frees);
}