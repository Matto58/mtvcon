#include <stdint.h>
#include <stdbool.h>
#pragma once

void *hvReadFile(int16_t partInx, char *fileName, uint64_t *readSize);
void hvWriteFile(int16_t partInx, char *fileName, void *data, uint64_t size);
bool hvInit();
