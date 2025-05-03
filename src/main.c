#include "hypervisor/hypervisor.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    // testing code, todo: add a proper ui?
    hypervisor_t *hv = hvInit("../vdisk.bin");
    if (hv == NULL) {
        printf("Failed to initialise hypervisor!\n");
        return 1;
    }
    size_t fileSize, bufferSize; 
    void *buffer = hvReadFile(hv, 0, "my file.txt", &fileSize, &bufferSize);
    if (buffer == NULL) {
        printf("Could not read file 'my file.txt'!\n");
        hvDestroy(hv);
        return 1;
    }
    char str[100] = {0};
    strncpy(str, buffer, fileSize);
    printf("Read %ld bytes. Content:\n%s\n", fileSize, str);
    free(buffer);

    char *newText = "mroew :3";
    if (!hvWriteFile(hv, 0, "my file.txt", newText, strlen(newText))) {
        printf("Could not write '%s' to my file.txt!\n", newText);
        hvDestroy(hv);
        return 1;
    }
    hvDestroy(hv);
    return 0;
}
