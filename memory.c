#include <stdlib.h>
#include "memory.h"
// malloc과 realloc을 하나의 함수로 전부 처리할 수 있다. 
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if(newSize==0) {
        free(pointer);
        return NULL;
    }
    void* result = realloc(pointer, newSize);
    if(result==NULL) exit(1);
    return result;
}