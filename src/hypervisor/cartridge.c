#include "hypervisor.h"
#include "../mmfs/dbg.h"
#include "../mmfs/mmfsutils.h"
#include <string.h>

cartridge_t *crInit(char *cartridgeId) {
    char *funcName = "crInit";
    cartridge_t *cr = dbgMalloc(sizeof(cartridge_t), funcName);
    char driveName[512];
    snprintf(driveName, 511, "cartridges/%s.bin", cartridgeId);
    cr->drive = fopen(driveName, "r+");
    if (cr->drive == NULL) {
        dbgFree(cr, funcName);
        return NULL;
    }

    uint64_t nameLength = 0, nameBufSize = 0;
    void *name = muReadFile(cr->drive, 0, "_name", &nameLength, &nameBufSize);
    if (name == NULL) {
        dbgLog("crInit: failed to read file '_name' from %s, falling back to '<unknown cartridge>'", driveName);
        char *nameStr = "<unknown cartridge>";
        name = dbgMalloc(nameLength + 1, funcName);
        nameLength = strlen(nameStr);
        strncpy(name, nameStr, nameLength + 1);
    }
    cr->name = name;

    uint64_t descLength = 0, descBufSize = 0;
    void *desc = muReadFile(cr->drive, 0, "_desc", &descLength, &descBufSize);
    if (desc == NULL) {
        dbgLog("crInit: failed to read file '_desc' from %s, falling back to an empty string", driveName);
        desc = dbgMalloc(1, funcName);
        descLength = 0;
        ((char *)desc)[0] = '\0';
    }
    cr->desc = desc;
}

void crDestroy(cartridge_t *cartridge) {
    if (cartridge == NULL) return;
    char *funcName = "crDestroy";
    fclose(cartridge->drive);
    dbgFree(cartridge->name, funcName);
    dbgFree(cartridge->desc, funcName);
    dbgFree(cartridge, funcName);
}

void *crGetMainProgram(hypervisor_t *parentHv, uint64_t *readSize, uint64_t *bufSize) {
    return NULL; // todo: implement after moving hvReadFile & hvWriteFile to mmfsutils.c
}
