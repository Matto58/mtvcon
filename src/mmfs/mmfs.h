#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#pragma once

bool mmfsGoToPartition(FILE *drive, int16_t index);
bool mmfsGetPartitionData(FILE *drive, uint64_t *serialNum, char *label);
bool mmfsFindFile(FILE *drive, char *shortFileName, uint8_t *permissions, uint64_t *lastMod, uint64_t *created, uint64_t *lastAcc, char *owner, uint64_t *size);
bool mmfsGetFileData(FILE *drive, uint8_t *permissions, uint64_t *lastMod, uint64_t *created, uint64_t *lastAcc, char *shortFileName, char *owner, uint64_t *size);
bool mmfsReadNextFileSector(FILE *drive, void *destination);
char *mmfsShortenFileName(char *str);
