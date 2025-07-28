#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define CON_VERSION "1.0.0"
#define HV_VERSION "0.1.0"
#define MMFS_VERSION "2507-1"

#pragma once

struct __cartridge_t {
    FILE *drive;
    char *name, *desc;
};
typedef struct __cartridge_t cartridge_t;
// todo: add more shit, this is p barebones for now
struct __hypervisor_t {
    FILE *drive;
    cartridge_t *mountedCart;
};
typedef struct __hypervisor_t hypervisor_t;

void *hvReadFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, uint64_t *readSize, uint64_t *bufSize);
bool hvWriteFile(hypervisor_t *hvCtx, int16_t partInx, char *fileName, void *data, uint64_t size);
hypervisor_t *hvInit(char *driveLocation);
void hvDestroy(hypervisor_t *hvCtx);
bool hvIsValid(hypervisor_t *hvCtx);

cartridge_t *crInit(char *cartridgeId);
void crDestroy(cartridge_t *cartridge);
void *crGetMainProgram(hypervisor_t *parentHv, uint64_t *readSize, uint64_t *bufSize);
