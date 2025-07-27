#include "hypervisor.h"
#include "../mmfs/mmfs.h"
#include "../mmfs/dbg.h"
#include <stdlib.h>

bool canWriteToNextSect(hypervisor_t *hv) {
    int c = fgetc(hv->drive);
    fseek(hv->drive, SEEK_CUR, -1);
    return c == 0;
}

bool hvIsValid(hypervisor_t *hv) {
    return hv != NULL && hv->drive != NULL;
}

uint64_t roundToFlSector(uint64_t n) {
    return (n / 127 + 1) * 127;
}
uint64_t roundToClassicSector(uint64_t n) {
    return (n / 128 + 1) * 128;
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
    if (!hvIsValid(hvCtx) || fileName == NULL || readSize == NULL || bufSize == NULL) return NULL;

    if (!mmfsGoToPartition(hvCtx->drive, partInx))
        return NULL;

    if (!mmfsFindFile(hvCtx->drive, mmfsShortenFileName(fileName), NULL, NULL, NULL, NULL, NULL, readSize))
        return NULL;

    long base = ftell(hvCtx->drive);
    uint64_t *dataPtrs = mmfsGetFileDataPtrs(hvCtx->drive, 0, NULL);
    fseek(hvCtx->drive, base, SEEK_SET);
    if (dataPtrs == NULL) return NULL;

    *bufSize = roundToFlSector(*readSize);
    char *funcname = "hvReadFile";
    void *buffer = dbgMalloc(*bufSize, funcname);
    // 127 is how many bytes of a file can be stored in a sector
    // 128 bytes per sector (first byte is 05 - file data marker)
    for (uint64_t i = 0; dataPtrs[i] != 0; i++) {
        fseek(hvCtx->drive, dataPtrs[i]*128, SEEK_SET);
        if (!mmfsReadNextFileSector(hvCtx->drive, buffer + i*127)) {
            dbgFree(buffer, funcname);
            dbgFree(dataPtrs, funcname);
            return NULL;
        }
    }
    dbgFree(dataPtrs, funcname);
    return buffer;
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
