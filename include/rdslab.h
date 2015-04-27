#ifndef _RDSLAB_H
#define _RDSLAB_H

#include <stdint.h>
#include "rdheap.h"
#include <stdbool.h>

// define the size of a slab in pages
#define RDSLAB_PAGES 2

typedef struct {
    uint64_t obj_size; // size of objects in this slab
    uint64_t max; // max number of objects
    uint64_t count; // number of objects in the slab
    rdheap_t *free;
    void *raw; // raw memory of the slab
} rdslab_t;


rdslab_t *rdslab_new(uint64_t obj_size);
void *rdslab_get_object(rdslab_t *slab, uint64_t obj_num);
void rdslab_free_object(rdslab_t *slab, uint64_t obj_num);
void *rdslab_next_free(rdslab_t *slab);
bool rdslab_contains_ptr(rdslab_t *slab, void *ptr);
uint64_t rdslab_object_for_ptr(rdslab_t *slab, void *ptr);
#endif
