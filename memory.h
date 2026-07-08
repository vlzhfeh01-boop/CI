#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"
// macro 
#define GROW_CAPACITY(capacity) \ 
    ((capacity) < 8 ? 8 : (capacity)*2)

// type크기에 따라서 실제 바이트 수가 달라진다. 
#define GROW_ARRAY(type, pointer, oldCount, newCount) \ 
    (type*)reallocate(pointer, sizeof(type)*(oldCount), \
    sizeof(type)*(newCount))

#define FREE_ARRAY(type,pointer,oldCount)\
    reallocate(pointer,sizeof(type)*(oldCount),0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif