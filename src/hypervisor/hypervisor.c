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
    if (!validHv(hvCtx)) return NULL;

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

bool hvWriteFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, void *data, uint64_t size) {
    if (!validHv(hvCtx)) return NULL;
    if (!mmfsGoToPartition(hvCtx->drive, partInx)) return NULL;
    uint64_t currentSize;
    if (!mmfsFindFile(hvCtx->drive, mmfsShortenFileName(fileName), NULL, NULL, NULL, NULL, NULL, &currentSize))
        return false;
    uint64_t remainingBytes = size;
    char blankFlSector[128] = {0};
    blankFlSector[0] = 0x05;
    uint64_t roundedSize = roundToFlSector(size), roundedCurrentSize = roundToFlSector(currentSize);
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
    // TODO: FIX THIS ELSE STATEMENT NOT WORKING & LEAKING MEMORY. VALGRIND GAVE 589221 ERRORS TO THIS. WHAT THE FUCK!?!?!?!
    else {
        // whoops, we have to shift the sectors to the left or to the right
        int sectShift = (roundedSize - roundedCurrentSize)/127;
        if (roundedSize < roundedCurrentSize) {
            // <-- left shift
            // here we write n-sectShift sectors of blank file data and a blank sector
            // yes, this is stupid, but hopefully mmfs2 will allow for a fix
            while (remainingBytes < 127*sectShift) {
                fwrite(blankFlSector, 1, 128, hvCtx->drive);
                remainingBytes -= 127;
            }
            char blankSect[] = {0};
            for (int i = 0; i < sectShift; i++)
                fwrite(blankSect, 1, 128, hvCtx->drive);
            return hvWriteFile(hvCtx, partInx, fileName, data, size);
        }
        else {
            // --> right shift
            // no comment. i looked away from this code for 2 mins and no longer have an idea how this works. help
            fseek(hvCtx->drive, roundedCurrentSize, SEEK_CUR);
            int iters = roundedCurrentSize/127;
            char sect[128];
            for (int i = 0; i < iters; i++) {
                fseek(hvCtx->drive, -128, SEEK_CUR);
                fread(sect, 1, 128, hvCtx->drive);
                if (mmfsValidatePartSector(sect))
                    return false;
                fwrite(sect, 1, 128, hvCtx->drive);
                fseek(hvCtx->drive, -256, SEEK_CUR);
            }
            return hvWriteFile(hvCtx, partInx, fileName, data, size);
        }
    }
    // negative cuz we're going back + the position of the size property to shift to it
    fseek(hvCtx->drive, -roundToClassicSector(size) + 0x78, SEEK_CUR);
    printf("seek to size prop, now at %ld\n", ftell(hvCtx->drive));
    fwrite(&size, 8, 1, hvCtx->drive);
    printf("write to size prop, now at %ld\n", ftell(hvCtx->drive));
    return true;
}
