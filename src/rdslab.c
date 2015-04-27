#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "rdheap.h"
#include "rdslab.h"



/**
 * Allocate a new slab
 */
rdslab_t *rdslab_new(uint64_t obj_size) {
    rdslab_t *new_slab = NULL;
    if((new_slab = malloc(sizeof(rdslab_t))) == NULL) {
        perror("malloc");
        return NULL;
    }
    uint64_t slab_size =  RDSLAB_PAGES * sysconf(_SC_PAGE_SIZE);
    if((new_slab->raw = malloc(slab_size)) == NULL) {
        perror("malloc");
        free(new_slab);
        return NULL;
    }
    new_slab->obj_size = obj_size;
    new_slab->max = slab_size / obj_size;
    new_slab->count = 0;

    // This next bit populates our free heap
    uint64_t *free_objects = NULL;
    if((free_objects = calloc(new_slab->max, sizeof(uint64_t))) == NULL) {
        perror("calloc");
        free(new_slab->raw);
        free(new_slab);
        return NULL;
    }
    for(uint64_t i=0; i<new_slab->max; i++) {
        free_objects[i] = i;
    }
    new_slab->free = rdheap_from_array(free_objects, new_slab->max);
    free(free_objects);
    return new_slab;
}

/**
 * Get the ptr to a portion of the slab that represents the object described
 * by the given index and size
 */
void *rdslab_get_object(rdslab_t *slab, uint64_t obj_num) {
    return slab->raw + (obj_num * slab->obj_size);
}

uint64_t rdslab_object_for_ptr(rdslab_t *slab, void *ptr) {
    if(rdslab_contains_ptr(slab, ptr)) {
        return (ptr - slab->raw) / slab->obj_size;
    }
    return 0;
}

void rdslab_free_object(rdslab_t *slab, uint64_t obj_num) {
    rdheap_insert(slab->free, obj_num);
    slab->count--;
}

/**
 * Get the next free object in the slab.
 */
void *rdslab_next_free(rdslab_t *slab) {
    slab->count++;
    return rdslab_get_object(slab, rdheap_pop(slab->free));
}

/**
 * Check if a ptr is in this slab
 */
bool rdslab_contains_ptr(rdslab_t *slab, void *ptr) {
    return ptr >= slab->raw && ptr < slab->raw + (slab->max * slab->obj_size);
}

