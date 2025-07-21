#include "hypervisor/hypervisor.h"
#include "mmfs/mmfsutils.h"
#include <stdlib.h>
#include <string.h>

#define TESTWRITE 0

// returns 0 on success, 1 on mild unsuccess, 2 on not mild unsuccess and with 3 you're just cooked
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Please specify a .bin with the MMFS-formatted system partition!\n");
        return 3;
    }
    if (argc > 2 && strcmp(argv[2], "__GENSYS") == 0)
        if (!muCreateDriveAndPartition(argv[1], "mtvcon system partition", 1024))
            printf("WARNING: Generating %s failed\n", argv[1]);
    // testing code, todo: add a proper ui?
    hypervisor_t *hv = hvInit(argv[1]);
    if (hv == NULL) {
        printf("Failed to initialise hypervisor!\n");
        return 2;
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

#ifdef TESTWRITE
#if TESTWRITE
    char *newText = "my testing file yippee!!";
    if (!hvWriteFile(hv, 0, "my file.txt", newText, strlen(newText))) {
        printf("Could not write '%s' to my file.txt!\n", newText);
        hvDestroy(hv);
        return 1;
    }
#endif
#endif
    hvDestroy(hv);
    return 0;
}
