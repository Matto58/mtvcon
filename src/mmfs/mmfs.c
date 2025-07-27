#include "mmfs.h"
#include "dbg.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

bool mmfsValidatePartSector(char *sector) {
    for (unsigned char i = 0; i < 128; i += 4) {
        // A1 B0 B9 12 is the magic number for MMFS partitions
        // a sector filled with it is the first sector of the partition
        if (sector[i+0] == (char)0xa1
            && sector[i+1] == (char)0xb0
            && sector[i+2] == (char)0xb9
            && sector[i+3] == (char)0x12) continue;
        return false;
    }
    return true;
}

bool mmfsGoToPartition(FILE *drive, int16_t index) {
    if (drive == NULL) return false;
    fseek(drive, 0, SEEK_SET);
    char buffer[128] = {0};
    int16_t currentIndex = -1;
    while (feof(drive) == 0) {
        size_t read = dbgFread(buffer, 1, 128, drive, "mmfsGoToPartition");
        if (read < 128) return false;
        if (mmfsValidatePartSector(buffer)) {
            currentIndex++;
            if (index == currentIndex) return true;
            continue;
        }
    }
    return false;
}

bool mmfsGetPartitionData(FILE *drive, uint64_t *serialNum, char *label) {
    if (drive == NULL) return false;
    char buffer[128] = {0};
    size_t read = dbgFread(buffer, 1, 128, drive, "mmfsGetPartitionData");
    if (read < 128) return false;
    // partition magic number validation
    // disgusting but can't be cleaned up i think
    if (buffer[0] != (char)0x02 // prototype marker
        || buffer[1] != (char)0x01 || buffer[2] != (char)0x00 // version
        || buffer[3] != (char)0x19 || buffer[4] != (char)0x07 || buffer[5] != (char)0x01 // prototype version
    ) return false;
    // validates key A and key B
    for (int i = 0; i < 24; i++)
        if (buffer[0x50 + i] != buffer[0x68 + i]) return false;
    if (serialNum != NULL) memcpy(serialNum, &buffer[6], 8);
    if (label != NULL) memcpy(label, &buffer[16], 48);
    return true;
}

// original c# implementation goes like this:
// public static string ShortFilenameify(string filename)
// {
//     int remainingChars = Math.Max(0, filename.Length - 46 - 1);
//     string remainingCharsStr = "~" + remainingChars.ToString();
//     string shortFilename = filename.Length <= 46 ? filename : filename[..Math.Min(filename.Length, 46 - remainingCharsStr.Length)];
//     if (remainingChars > 0) shortFilename += remainingCharsStr;
//     return shortFilename;
// }
int max(int a, int b) { return a > b ? a : b; }
int min(int a, int b) { return a < b ? a : b; }
int numToStrlen(size_t n) { // gets length of a number when converted to a string
    int i = 1;
    while (n > 10) { n /= 10; i++; }
    return i;
}
char *mmfsShortenFileName(char *str) {
    if (str == NULL) return NULL;
    static char name[46] = {0};
    size_t length = strlen(str);
    // subtracting by 45 (max short file name length)...
    int remainingChars = max(0, length - 45);
    // ...then adding space for the digit
    remainingChars = max(0, remainingChars - numToStrlen(remainingChars));
    if (length <= 45) strncpy(name, str, length);
    else {
        // subtracting 1 because tilde
        int needToCopy = length - remainingChars - 1;
        snprintf(name, 46, "%.*s~%d", needToCopy, str, remainingChars);
    }
    return name;
}

bool mmfsGetFileMetadata(FILE *drive, uint8_t *permissions, uint64_t *lastMod, uint64_t *created, uint64_t *lastAcc, char *shortFileName, char *owner, uint64_t *size) {
    if (drive == NULL) return false;
    char sector[128];
    dbgFread(sector, 1, 128, drive, "mmfsGetFileMetadata");
    if (sector[0] != (char)3) return false;

    if (permissions != NULL) memcpy(permissions, &sector[1], 1);
    if (lastMod != NULL) memcpy(lastMod, &sector[0x02], 8);
    if (created != NULL) memcpy(created, &sector[0x0a], 8);
    if (lastAcc != NULL) memcpy(lastAcc, &sector[0x70], 8);
    if (shortFileName != NULL) memcpy(shortFileName, &sector[0x12], 46);
    if (owner != NULL) memcpy(owner, &sector[0x40], 48);
    if (size != NULL) memcpy(size, &sector[0x78], 8);
    return true;
}

// todo: add partition index to this function
bool mmfsFindFile(FILE *drive, char *shortFileName, uint8_t *permissions, uint64_t *lastMod, uint64_t *created, uint64_t *lastAcc, char *owner, uint64_t *size) {
    if (drive == NULL) return false;
    char foundName[46];
    fseek(drive, 0, SEEK_SET);
    while (!feof(drive)) {
        bool foundData = mmfsGetFileMetadata(drive, permissions, lastMod, created, lastAcc, foundName, owner, size);
        if (foundData && strncmp(shortFileName, foundName, min(strlen(shortFileName), strlen(foundName))) == 0)
            return true;
    }
    return false;
}

bool mmfsReadNextFileSector(FILE *drive, void *destination) {
    if (drive == NULL) return false;
    char sector[128];
    dbgFread(sector, 1, 128, drive, "mmfsReadNextFileSector");
    if (sector[0] != (char)5) return false;
    memcpy(destination, &sector[1], 127);
    return true;
}

// NOTE FOR USER: CALL free ON OUTPUTTED BUFFER WHEN YOU'RE DONE!!!!!
uint64_t *mmfsGetFileDataPtrs(FILE *drive, uint64_t continueAddr, int *iter) {
    if (drive == NULL) return NULL;
    char sector[128];
    char *hiMyNameIs = "mmfsGetFileDataPtrs";
    dbgFread(sector, 1, 128, drive, hiMyNameIs);
    if (sector[0] != (char)6) return NULL; // todo: add error messages
    int iterN = 1;
    if (iter == NULL) iter = &iterN;
    uint64_t *ptrs = (uint64_t *)dbgMalloc(120, hiMyNameIs); // 15 pointers * 8 bytes per pointer = 120
    for (int i = 0; i < 15; i++) {
        uint64_t ptr;
        memcpy(&ptr, sector + (i+1)*8, 8);
        ptrs[i] = ptr;
    }
    uint64_t continueAddrNest = 0;
    memcpy(&continueAddrNest, &sector[1], 7);
    if (continueAddrNest == 0) return ptrs;

    uint64_t *ptrsNew = mmfsGetFileDataPtrs(drive, continueAddrNest, &iterN);
    if (ptrsNew == NULL) return ptrs;
    uint64_t *ptrsFinal = (uint64_t *)dbgMalloc(8 * 15 * (iterN+1), hiMyNameIs);
    memcpy(ptrsFinal, ptrs, 8*15);
    memcpy(ptrsFinal + 8*15, ptrsNew, 8 * 15 * iterN);
    dbgFree(ptrs, hiMyNameIs);
    dbgFree(ptrsNew, hiMyNameIs);
    return ptrsFinal;
}

bool mmfsSetFileDataPtrs(FILE *drive, uint64_t *ptrs, int ptrCount) {
    if (drive == NULL) return false;
    char sector[128];
    char *hiMyNameIs = "mmfsSetFileDataPtrs"; // lame ass joke (chika chika slim shady)
    dbgFread(sector, 1, 128, drive, hiMyNameIs);
    if (sector[0] != (char)6) return false;
    
    int i;
    fseek(drive, 8, SEEK_CUR);
    for (i = 0; i < 15 && i < ptrCount; i++)
        fwrite(&ptrs[i], 8, 1, drive);
    // snap to partition
    fseek(drive, 8 * (14-i), SEEK_CUR);

    uint64_t continueAddr = 0;
    memcpy(&continueAddr, &sector[1], 7);
    if (continueAddr != 0) {
        fseek(drive, continueAddr * 128, SEEK_SET);
        return mmfsSetFileDataPtrs(drive, &ptrs[15], ptrCount-15);
    }
    if (i == 14 && ptrCount > 15 && continueAddr == 0) {
        // find blank sector we can write to
        long base = ftell(drive);
        do {
            dbgFread(sector, 1, 128, drive, hiMyNameIs);
            if (feof(drive)) return false;
        } while (sector[0] != 0);

        char blankPtrSect[128] = {0}; blankPtrSect[0] = 6;
        fseek(drive, -128, SEEK_CUR);
        continueAddr = ftell(drive)/128;
        memcpy(&blankPtrSect[1], (void *)(&continueAddr) + 1, 7);
        dbgFwrite(blankPtrSect, 1, 128, drive, hiMyNameIs);

        bool result = mmfsSetFileDataPtrs(drive, &ptrs[15], ptrCount-15);
        fseek(drive, base, SEEK_SET);
        return result;
    }
    return true;
}
