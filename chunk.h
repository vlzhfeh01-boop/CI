#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"

typedef enum{
    OP_RETURN,
} OpCode;

typedef struct{
    int count; // number of elements in the array we have allocated.
    int capacity; // how many of those allocated entries are actually in use.
    uint8_t* code; // pointer, array
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte);

#endif