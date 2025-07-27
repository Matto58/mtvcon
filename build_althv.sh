#!/bin/bash
gcc -g src/main.c src/mmfs/dbg.c src/hypervisor_alt/mmfsutils_spoof.c src/hypervisor_alt/hypervisor.c src/ui/mainmenu.c -o mtvcon_althv