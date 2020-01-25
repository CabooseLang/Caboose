#ifndef caboose_memory_h
#define caboose_memory_h

#include "object.h"

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)
#define GROW_ARRAY(previous, type, oldCount, count)                            \
    (type*)reallocate(                                                         \
      previous, sizeof(type) * (oldCount), sizeof(type) * (count))
#define FREE_ARRAY(type, pointer, oldCount)                                    \
    reallocate(pointer, sizeof(type) * (oldCount), 0)
#define ALLOCATE(type, count) (type*)reallocate(NULL, 0, sizeof(type) * (count))
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

void*
reallocate(void* previous, size_t oldSize, size_t newSize);

void
freeObjects();

void
collectGarbage();

void
markValue(Value value);

void
markObject(Obj* object);

#endif
