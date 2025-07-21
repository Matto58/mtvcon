#include "mmfsutils.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

bool muCreateDriveAndPartition(char *filename, char *label, uint64_t sectorCount) {
    FILE *drive = fopen(filename, "w+");
    if (drive == NULL) return false;

    srand(time(NULL));

    char blank[128] = {0};
    for (uint64_t i = 0; i < sectorCount; i++)
        fwrite(blank, 1, 128, drive);

    fseek(drive, 0, SEEK_SET);
    char partMarker[] = { 0xa1, 0xb0, 0xb9, 0x12 };
    for (int i = 0; i < 128; i += 4) fwrite(partMarker, 1, 4, drive);
    char partMeta[] = { 2, 1, 0, 25, 7, 1 };
    fwrite(partMeta, 1, 6, drive);
    uint64_t serialNumber = rand() | ((uint64_t)rand() << 32);
    fwrite(&serialNumber, 1, 8, drive);
    fseek(drive, 2, SEEK_CUR);

    char labelFinal[0x40] = {0};
    strncpy(labelFinal, label, 0x3f);
    fwrite(labelFinal, 1, 0x40, drive);

    for (int i = 0; i < 0x18; i += 4) {
        int n = rand();
        fwrite(&n, 1, 4, drive);
        fseek(drive, 0x14, SEEK_CUR);
        fwrite(&n, 1, 4, drive);
        fseek(drive, -0x18, SEEK_CUR);
    }
    
    fclose(drive);
    return true;
}