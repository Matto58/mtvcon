#include "mmfs.h"

bool muCreateDriveAndPartition(char *filename, char *label, uint64_t sectorCount);
void *muReadFile(FILE *drive, int16_t partInx, char *fileName, uint64_t *readSize, uint64_t *bufSize);