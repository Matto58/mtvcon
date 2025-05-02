#include "hypervisor/hypervisor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    // testing code, todo: add a proper ui?
    if (!hvInit()) {
        printf("Failed to initialise hypervisor!\n");
        return 1;
    }
    size_t fileSize;
    void *buffer = hvReadFile(0, "my file.txt", &fileSize);
    if (buffer == NULL) {
        printf("Could not read file 'my file.txt'!\n");
        return 1;
    }
    char str[100] = {0};
    strncpy(str, buffer, fileSize);
    printf("Read %ld bytes. Content:\n%s\n", fileSize, str);
    free(buffer);
    return 0;
}
