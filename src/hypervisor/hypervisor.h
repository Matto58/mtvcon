#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#pragma once

// todo: add more shit, this is p barebones for now
typedef struct {
    FILE *drive;
} hypervisor_t;

void *hvReadFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, uint64_t *readSize, uint64_t *bufSize);
bool hvWriteFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, void *data, uint64_t size);
hypervisor_t *hvInit(char *driveLocation);
void hvDestroy(hypervisor_t *hvCtx);
