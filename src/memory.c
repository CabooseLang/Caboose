#include <stdlib.h>

#include "memory.h"
#include "common.h"

void* reallocate(void* previous, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    free(previous);
    return NULL;
  }

  return realloc(previous, newSize);
}