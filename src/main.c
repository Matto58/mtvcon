#include "mmfs/mmfsutils.h"
#include "ui/mainmenu.h"
#include "mmfs/dbg.h"
#include <string.h>
#include <stdlib.h>

int checkForFlag(char **argv, char *flag) {
    size_t len = strlen(flag);
    for (int i = 1; argv[i] != NULL; i++)
        if (strncmp(argv[i], flag, len) == 0)
            return i;
    return 0;
}

// returns 0 on success, 1 on mild unsuccess, 2 on not mild unsuccess and with 3 you're just cooked
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Please specify a .bin with the MMFS-formatted system partition!\n");
        return 3;
    }
    int partFlagInx = checkForFlag(argv, "__GENSYS");
    if (partFlagInx != 0) {
        size_t partSize = 
            strlen(argv[partFlagInx]) > 8
            ? atoll(&argv[partFlagInx][8])
            : 1024; // default is 1024 sectors
        if (!muCreateDriveAndPartition(argv[1], "mtvcon system partition", partSize))
            printf("WARNING: Generating %s failed\n", argv[1]);
    }
    if (checkForFlag(argv, "__DEBUG")) dbgSetDebug(1);
    return mainMenu(argv[1]);
}
