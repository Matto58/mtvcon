#include "hypervisor.h"
#include "../mmfs/mmfs.h"
#include <stdlib.h>

bool canWriteToNextSect(hypervisor_t *hv) {
    int c = fgetc(hv->drive);
    fseek(hv->drive, SEEK_CUR, -1);
    return c == 0;
}

bool validHv(hypervisor_t *hv) {
    return hv != NULL && hv->drive != NULL;
}

uint64_t roundToFlSector(uint64_t n) {
    return (n / 127 + 1) * 127;
}
uint64_t roundToClassicSector(uint64_t n) {
    return (n / 128 + 1) * 128;
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
    if (!validHv(hvCtx) || fileName == NULL || readSize == NULL || bufSize == NULL) return NULL;

    if (!mmfsGoToPartition(hvCtx->drive, partInx))
        return NULL;

    if (!mmfsFindFile(hvCtx->drive, mmfsShortenFileName(fileName), NULL, NULL, NULL, NULL, NULL, readSize))
        return NULL;

    *bufSize = roundToFlSector(*readSize);
    void *buffer = malloc(*bufSize);
    // 127 is how many bytes of a file can be stored in a sector
    // 128 bytes per sector (first byte is 05 - file data marker)
    for (uint64_t i = 0; i < *readSize; i += 127) {
        if (!mmfsReadNextFileSector(hvCtx->drive, buffer + i)) return NULL;
    }
    return buffer;
}

// TODO: THIS SHIT IS BROKEN. FIX AFTER REWRITING THE SPEC.
bool hvWriteFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, void *data, uint64_t size) {
    if (!validHv(hvCtx) || fileName == NULL || data == NULL) return NULL;
    if (!mmfsGoToPartition(hvCtx->drive, partInx)) return NULL;
    uint64_t currentSize;
    if (!mmfsFindFile(hvCtx->drive, mmfsShortenFileName(fileName), NULL, NULL, NULL, NULL, NULL, &currentSize))
        return false;
    uint64_t remainingBytes = size;
    char blankFlSector[128] = {0};
    blankFlSector[0] = 0x05;
    uint64_t roundedSize = roundToFlSector(size), roundedCurrentSize = roundToFlSector(currentSize);
    long basePos = ftell(hvCtx->drive);
    if (roundedSize == roundedCurrentSize) {
        // fits within the existing sector(s)!! we can write without shifting the rest of the sectors
        while (true) {
            printf("blankFlSector before write, now at %ld\n", ftell(hvCtx->drive));
            fwrite(blankFlSector, 1, 128, hvCtx->drive);
            printf("blankFlSector write, now at %ld\n", ftell(hvCtx->drive));
            fseek(hvCtx->drive, -127, SEEK_CUR);
            printf("blankFlSector seek, now at %ld\n", ftell(hvCtx->drive));
            fwrite(data + (size - remainingBytes), 1, remainingBytes > 127 ? 127 : remainingBytes, hvCtx->drive);
            printf("data write, now at %ld\n", ftell(hvCtx->drive));
            if (remainingBytes < 127) {
                fseek(hvCtx->drive, -size - roundedSize/127, SEEK_CUR); // align to first file sector
                printf("first file sector alignment, now at %ld\n", ftell(hvCtx->drive));
                break;
            }
            remainingBytes -= 127;
        }
    }
    else {
        // whoops, we have to shift the sectors to the left or to the right
        int dataSects = roundedSize/127, newDataSects = roundedCurrentSize/127;
        int sectShift = newDataSects - dataSects;
        // DEBUG SHIT
        printf("original file sect count: %d, new file sect count: %d -- DIFFERENCE: %d\n", dataSects, newDataSects, sectShift);
        printf("basePos = %ld\n", basePos);
        // ofc validate if we can even write
        for (int i = 0; i < newDataSects + sectShift; i++) {
            char sect[128];
            fseek(hvCtx->drive, basePos + i * 128, SEEK_SET);
            printf("WRITE VALIDATION: seeked to sect start, pos: %ld\n", ftell(hvCtx->drive));
            fread(sect, 1, 128, hvCtx->drive);
            printf("WRITE VALIDATION: after fread, pos: %ld\n", ftell(hvCtx->drive));
            if (mmfsValidatePartSector(sect))
                return false;
            printf("WRITE VALIDATION: sector validated!\n");
        }
        mmfsShiftSectors(hvCtx->drive, ftell(hvCtx->drive), sectShift, newDataSects, blankFlSector);
        printf("SHIFT: (postshift) final pos: %ld\n", ftell(hvCtx->drive));
    }
    fseek(hvCtx->drive, basePos - 8, SEEK_SET);
    printf("seek to size prop, now at %ld\n", ftell(hvCtx->drive));
    fwrite(&size, 8, 1, hvCtx->drive);
    printf("write to size prop, now at %ld\n", ftell(hvCtx->drive));
    return true;
}
