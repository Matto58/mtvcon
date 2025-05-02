#include "hypervisor.h"
#include "../mmfs/mmfs.h"
#include <stdlib.h>

// todo: implement cartridge_t and hypervisor_t as mentioned in HypervisorFunctions.txt
FILE *globalHvDrive;

bool canWriteToNextSect() {
    int c = fgetc(globalHvDrive);
    fseek(globalHvDrive, SEEK_CUR, -1);
    return c == 0;
}

bool hvInit() {
    globalHvDrive = fopen("../vdisk.bin", "r+");
    if (globalHvDrive == NULL) return false;
    return true;
}

void *hvReadFile(int16_t partInx, char *fileName, uint64_t *readSize) {
    if (globalHvDrive == NULL)
        return NULL;

    if (!mmfsGoToPartition(globalHvDrive, partInx))
        return NULL;

    if (!mmfsFindFile(globalHvDrive, mmfsShortenFileName(fileName), NULL, NULL, NULL, NULL, NULL, readSize))
        return NULL;

    void *buffer = malloc(((*readSize) / 127) * 127 + 1);
    // 127 is how many bytes of a file can be stored in a sector
    // 128 bytes per sector (first byte is 05 - file data marker)
    for (uint64_t i = 0; i < *readSize; i += 127) {
        if (!mmfsReadNextFileSector(globalHvDrive, &buffer[i])) return NULL;
    }
    return buffer;
}

void hvWriteFile(int16_t partInx, char *fileName, void *data, uint64_t size) {
    // todo implement
}
