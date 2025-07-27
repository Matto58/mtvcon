#!/bin/bash
gcc -g src/main.c src/mmfs/dbg.c src/mmfs/mmfs.c src/mmfs/mmfsutils.c src/hypervisor/hypervisor.c src/ui/mainmenu.c -o mtvcon