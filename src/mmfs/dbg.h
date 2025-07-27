#include <stdio.h>
void dbgLog(const char *fmt, ...);
void dbgSetDebug(int val);
void *dbgMalloc(size_t size, char *sourceFunc);
void dbgFree(void *ptr, char *sourceFunc);
size_t dbgFread(void *buf, size_t size, size_t n, FILE *f, char *sourceFunc);
size_t dbgFwrite(void *buf, size_t size, size_t n, FILE *f, char *sourceFunc);
void dbgPrintMemStats();