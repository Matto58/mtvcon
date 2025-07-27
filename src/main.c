#include "mmfs/mmfsutils.h"
#include "ui/mainmenu.h"
#include <string.h>

// returns 0 on success, 1 on mild unsuccess, 2 on not mild unsuccess and with 3 you're just cooked
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Please specify a .bin with the MMFS-formatted system partition!\n");
        return 3;
    }
    if (argc > 2 && strcmp(argv[2], "__GENSYS") == 0)
        if (!muCreateDriveAndPartition(argv[1], "mtvcon system partition", 1024))
            printf("WARNING: Generating %s failed\n", argv[1]);
    return mainMenu();
}
