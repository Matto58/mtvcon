#include "hypervisor.h"
#include "../mmfs/mmfs.h"
#include <stdlib.h>

bool canWriteToNextSect(hypervisor_t *hv) {
    int c = fgetc(hv->drive);
    fseek(hv->drive, SEEK_CUR, -1);
    return c == 0;
}

hypervisor_t *hvInit(char *driveLocation) {
    hypervisor_t *hvCtx = malloc(sizeof(hypervisor_t));
    hvCtx->drive = fopen(driveLocation, "r+");
    if (hvCtx->drive == NULL) {
        free(hvCtx);
        return NULL;
    }
    return hvCtx;
}

void hvDestroy(hypervisor_t *hvCtx) {
    fclose(hvCtx->drive);
    free(hvCtx);
}

void *hvReadFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, uint64_t *readSize, uint64_t *bufSize) {
    if (hvCtx->drive == NULL)
        return NULL;

    if (!mmfsGoToPartition(hvCtx->drive, partInx))
        return NULL;

    if (!mmfsFindFile(hvCtx->drive, mmfsShortenFileName(fileName), NULL, NULL, NULL, NULL, NULL, readSize))
        return NULL;

    *bufSize = (*readSize / 127 + 1) * 127;
    void *buffer = malloc(*bufSize);
    // 127 is how many bytes of a file can be stored in a sector
    // 128 bytes per sector (first byte is 05 - file data marker)
    for (uint64_t i = 0; i < *readSize; i += 127) {
        if (!mmfsReadNextFileSector(hvCtx->drive, buffer + i)) return NULL;
    }
    return buffer;
}

void hvWriteFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, void *data, uint64_t size) {
    // todo implement
}
