// THIS FILE IS PART OF THE ALTERNATE HYPERVISOR.
// This means that all implementations here are working with the native file system and not MMFS.
// The current MMFS implementation is wonky and needs to be worked on.
// So in the meantime, the alpha versions of mtvcon will utilize this HV implementation.

#include "hypervisor.h"
#include "../mmfs/dbg.h"
#include <sys/stat.h>
#include <string.h>

// affixes .d at the end of a file name, note: make sure to free the buffer!!!
void *getD(char *drive) {
    size_t driveLen = strlen(drive);
    char *driveDir = dbgMalloc(driveLen + 3, "getD"); // +2 (for .d affix) +1 (for null byte)
    strcpy(driveDir, drive);
    driveDir[driveLen] = '.'; driveDir[driveLen+1] = 'd'; driveDir[driveLen+2] = 0;
    return driveDir;
}

void *hvReadFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, uint64_t *readSize, uint64_t *bufSize) {
    if (fileName == NULL || readSize == NULL || bufSize == NULL) return NULL;
    char n[512] = {0};
    snprintf(n, 511, "vdisk.bin.d/%s", fileName);
    FILE *f = fopen(n, "r");
    if (f == NULL) return NULL;
    struct stat s; stat(n, &s);
    *readSize = s.st_size;
    *bufSize = (s.st_size / 127 + 1) * 127;
    char *funcname = "hvReadFile";
    void *v = dbgMalloc(*bufSize, funcname);
    dbgFread(v, 1, *readSize, f, funcname);
    fclose(f);
    return v;
}
bool hvWriteFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, void *data, uint64_t size) {
    if (fileName == NULL || data == NULL) return NULL;
    char n[512] = {0};
    snprintf(n, 511, "vdisk.bin.d/%s", fileName);
    FILE *f = fopen(n, "w");
    if (f == NULL) return false;
    dbgFwrite(data, 1, size, f, "hvWriteFile");
    fclose(f);
    return true;
}
hypervisor_t *hvInit(char *driveLocation) {
    void *d = getD(driveLocation);
    // printf("%s", (char *)d);
    struct stat s;
    if (!(stat(d, &s) == 0 && S_ISDIR(s.st_mode))) mkdir(d, 0755);
    dbgFree(d, "hvInit");
    return (hypervisor_t *)1; // hack to not fail the null check
}
void hvDestroy(hypervisor_t *hvCtx) { return; }
bool hvIsValid(hypervisor_t *hvCtx) { return true; } // always true, because this is a stub implementation