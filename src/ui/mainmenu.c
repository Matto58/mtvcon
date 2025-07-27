#include "mainmenu.h"
#include "../hypervisor/hypervisor.h"
#include <stdlib.h>
#include <string.h>

char *dupeThatShit(char c, int repete) { // i am too lazy to think of a better name
    char *str = malloc(repete + 1);
    if (str == NULL) return NULL;
    for (int i = 0; i < repete; i++) str[i] = c;
    str[repete] = '\0';
    return str;
}

void loadCart() {
    // todo: create cartridge_t and load it here
    printf("not implemented yet!\n");
}

int mainMenu() {
    char *separator = dupeThatShit('=', 50);
    char *actions[] = {
        "load cartridge",
        "[dev] test file r/w",
        "quit",
        NULL
    };
    int exitcode = -1;
    while (exitcode == -1) {
        printf(
            "%s\n"
            " mtvcon v%s - hypervisor v%s - mmfs v%s\n"
            "%s\n",
            separator,
            CON_VERSION,
            HV_VERSION,
            MMFS_VERSION,
            separator
        );
        for (int i = 0; actions[i] != NULL; i++)
            printf("[%d] %s\n", i+1, actions[i]);
        printf("%s\n", separator);
        printf("> ");
        char input[3];
        scanf("%2s", input);
        int choice = atoi(input);        
        if (choice == 1) loadCart();
        else if (choice == 2) {
            hypervisor_t *hv = hvInit("vdisk.bin");
            if (!hvIsValid(hv)) {
                printf("Failed to initialise hypervisor!\n");
                continue;
            }

            printf("Reading 'my file.txt'...\n");
            uint64_t readSize = 0, bufSize = 0;
            void *buffer = hvReadFile(hv, 0, "my file.txt", &readSize, &bufSize);
            if (buffer == NULL) {
                printf("Read failed!\n");
                continue;
            }
            printf("Read %ld bytes into a buffer of size %ld. Content:\n%s\n", readSize, bufSize, (char *)buffer);

            printf("Writing 'my other file.txt'...\n");
            char *content = "all mine towers crumble down the flowers gasping under rubble";
            bool writeSuccess = hvWriteFile(hv, 0, "my other file.txt", content, strlen(content));
            printf("Wrote %ld bytes: %s\n", strlen(content), writeSuccess ? "success!" : "failed!");

            printf("And now reading 'my other file.txt' to make sure it really worked...\n");
            readSize = 0; bufSize = 0;
            free(buffer); // we are good people and we free our buffers! hip hip hooray!
            buffer = hvReadFile(hv, 0, "my other file.txt", &readSize, &bufSize);
            if (buffer == NULL)
                printf("Read failed!\n");
            else {
                printf("Read %ld bytes into a buffer of size %ld. Content:\n%s\n", readSize, bufSize, (char *)buffer);
                free(buffer);
            }
            printf("All done!\n");
            hvDestroy(hv); // god i love the word destroy it sounds so badass for something rather lame
        }
        else if (choice == 3) {
            printf("bye bye!! ^w^ owo\n");
            exitcode = 0;
        }
        // yes, we ignore incorrect choices
    }
    free(separator);
    return 0;
}