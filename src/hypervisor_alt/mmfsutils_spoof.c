#include <stdbool.h>
#include <stdint.h>

// this is an implementation of the mmfsutils.h header file which doesn't use mmfs
// so these are all stubs that don't do anything
// it exists so that the alt hypervisor can compile without mmfs in case it's like really buggy or something
// cuz that happened. uhh there was like a shitty way to write files that fit into more or less sectors than the active one
// it had to like shift around n shit and it was buggy so i made file data pointers a thing

bool muCreateDriveAndPartition(char *filename, char *label, uint64_t sectorCount) {
    return false;
}