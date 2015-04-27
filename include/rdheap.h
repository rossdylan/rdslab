#ifndef _RDHEAP_H
#define _RDHEAP_H

#include <stdint.h>

typedef struct {
    uint64_t length; // number of elements in heap
    uint64_t size; // array size
    uint64_t *array;
} rdheap_t;

void rdheap_print(rdheap_t *heap);

int rdheap_insert(rdheap_t *heap, uint64_t value);
uint64_t rdheap_pop(rdheap_t *heap);
rdheap_t *rdheap_new(const uint64_t size);
rdheap_t *rdheap_from_array(uint64_t array[], const uint64_t size);

#endif
