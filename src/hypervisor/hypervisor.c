#include "hypervisor.h"
#include "../mmfs/mmfs.h"
#include "../mmfs/dbg.h"
#include "../mmfs/mmfsutils.h"
#include <stdlib.h>

bool canWriteToNextSect(hypervisor_t *hv) {
    int c = fgetc(hv->drive);
    fseek(hv->drive, SEEK_CUR, -1);
    return c == 0;
}

bool hvIsValid(hypervisor_t *hv) {
    return hv != NULL && hv->drive != NULL;
}

hypervisor_t *hvInit(char *driveLocation) {
    char *funcname = "hvInit";
    hypervisor_t *hvCtx = dbgMalloc(sizeof(hypervisor_t), funcname);
    hvCtx->drive = fopen(driveLocation, "r+");
    if (hvCtx->drive == NULL) {
        dbgFree(hvCtx, funcname);
        return NULL;
    }
    return hvCtx;
}

void hvDestroy(hypervisor_t *hvCtx) {
    fclose(hvCtx->drive);
    dbgFree(hvCtx, "hvDestroy");
}

// todo: copy this to mmfsutils.c and use that function here
void *hvReadFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, uint64_t *readSize, uint64_t *bufSize) {
    if (!hvIsValid(hvCtx)) return NULL;
    return muReadFile(hvCtx->drive, partInx, fileName, readSize, bufSize);
}

// todo: implement in mmfsutils.c and use that function here
bool hvWriteFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, void *data, uint64_t size) {
    return false;
    /*
    if (!validHv(hvCtx) || fileName == NULL || data == NULL) return false;
    uint64_t currentSize;
    uint64_t *dataPtrs;
    if (!mmfsGoToPartition(hvCtx->drive, partInx)) return false;
    if (!mmfsFindFile(hvCtx->drive, mmfsShortenFileName(fileName), NULL, NULL, NULL, NULL, NULL, &currentSize))
        return false;
    if (!(dataPtrs = mmfsGetFileDataPtrs()))
    uint64_t remainingBytes = size;
    char blankFlSector[128] = {0};
    blankFlSector[0] = 0x05;
    uint64_t roundedSize = roundToFlSector(size), roundedCurrentSize = roundToFlSector(currentSize);
    return true;
    */
}
