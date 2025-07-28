#include "mmfsutils.h"
#include "dbg.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

bool muCreateDriveAndPartition(char *filename, char *label, uint64_t sectorCount) {
    FILE *drive = fopen(filename, "w+");
    if (drive == NULL) return false;

    char *hiMyNameIs = "muCreateDriveAndPartition";

    srand(time(NULL));

    char blank[128] = {0};
    for (uint64_t i = 0; i < sectorCount; i++)
        dbgFwrite(blank, 1, 128, drive, hiMyNameIs);

    fseek(drive, 0, SEEK_SET);
    char partMarker[] = { 0xa1, 0xb0, 0xb9, 0x12 };
    for (int i = 0; i < 128; i += 4) fwrite(partMarker, 1, 4, drive);
    char partMeta[] = { 2, 1, 0, 25, 7, 1 };
    dbgFwrite(partMeta, 1, 6, drive, hiMyNameIs);
    uint64_t serialNumber = rand() | ((uint64_t)rand() << 32);
    dbgFwrite(&serialNumber, 1, 8, drive, hiMyNameIs);
    fseek(drive, 2, SEEK_CUR);

    char labelFinal[0x40] = {0};
    strncpy(labelFinal, label, 0x3f);
    dbgFwrite(labelFinal, 1, 0x40, drive, hiMyNameIs);

    for (int i = 0; i < 0x18; i += 4) {
        int n = rand();
        dbgFwrite(&n, 1, 4, drive, hiMyNameIs);
        fseek(drive, 0x14, SEEK_CUR);
        dbgFwrite(&n, 1, 4, drive, hiMyNameIs);
        fseek(drive, -0x18, SEEK_CUR);
    }
    
    fclose(drive);
    return true;
}

uint64_t roundToFlSector(uint64_t n) {
    return (n / 127 + 1) * 127;
}
uint64_t roundToClassicSector(uint64_t n) {
    return (n / 128 + 1) * 128;
}

void *muReadFile(FILE *drive, int16_t partInx, char *fileName, uint64_t *readSize, uint64_t *bufSize) {
    if (!mmfsGoToPartition(drive, partInx))
        return NULL;
    if (!mmfsFindFile(drive, mmfsShortenFileName(fileName), NULL, NULL, NULL, NULL, NULL, readSize))
        return NULL;

    long base = ftell(drive);
    uint64_t *dataPtrs = mmfsGetFileDataPtrs(drive, 0, NULL);
    fseek(drive, base, SEEK_SET);
    if (dataPtrs == NULL) return NULL;

    *bufSize = roundToFlSector(*readSize);
    char *funcname = "muReadFile";
    void *buffer = dbgMalloc(*bufSize, funcname);
    // 127 is how many bytes of a file can be stored in a sector
    // 128 bytes per sector (first byte is 05 - file data marker)
    for (uint64_t i = 0; dataPtrs[i] != 0; i++) {
        fseek(drive, dataPtrs[i]*128, SEEK_SET);
        if (!mmfsReadNextFileSector(drive, buffer + i*127)) {
            dbgFree(buffer, funcname);
            dbgFree(dataPtrs, funcname);
            return NULL;
        }
    }
    dbgFree(dataPtrs, funcname);
    return buffer;
}
